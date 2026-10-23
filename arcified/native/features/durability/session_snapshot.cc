#include "session_snapshot.h"

#include <map>
#include <set>
#include <type_traits>

namespace arcified::durability {
namespace {

using namespace lifecycle;
constexpr std::string_view kMagic = "ARCSESS\n";
constexpr std::size_t kHeaderSize = 16;  // Magic and two uint32 fields.
constexpr std::size_t kChecksumSize = sizeof(std::uint32_t);

Failure Invalid(std::string context) {
  return {Error::kInvalidGraph, std::move(context)};
}

std::optional<Failure> Admit(const DurableSnapshot& snapshot,
                             const SnapshotAdmission& admission) {
  if (!admission.unsupported_feature_payloads_absent)
    return Failure{Error::kUnsupportedFeatures,
                   "Space metadata/split presentation absence not established"};
  if (!admission.snapshot_may_persist || !admission.profile_may_persist ||
      !admission.page_may_persist)
    return Failure{Error::kPersistenceNotAdmitted,
                   "native context eligibility checks required"};
  if (auto failure = admission.snapshot_may_persist(snapshot))
    return failure;
  for (const auto& profile : snapshot.profiles)
    if (!admission.profile_may_persist(profile))
      return Failure{Error::kPersistenceNotAdmitted,
                     "Profile context not admitted"};
  for (const auto& page : snapshot.pages)
    if (!admission.page_may_persist(page.id, page.profile))
      return Failure{Error::kPersistenceNotAdmitted,
                     "page context not admitted"};
  return std::nullopt;
}

// Use the single lifecycle model to enforce ownership and reference invariants.
// Validation-only navigation is discarded with this private model. It is never
// encoded, returned as restored state, or reported to native browser machinery.
Result<Model> BuildModel(
    const DurableSnapshot& snapshot,
    const std::map<PageId, RestoredNavigation>* navigation) {
  if (snapshot.schema_version != kSchemaVersion)
    return Failure{Error::kUnsupportedSchema, "unsupported workspace schema"};
  if (CheckContractVersion(snapshot.contract_version) != Status::kOk)
    return Failure{Error::kUnsupportedContract,
                   "unsupported lifecycle contract"};
  Model model;
  for (const auto& id : snapshot.profiles)
    if (model.AddProfile(id) != Status::kOk)
      return Invalid("Profile identity");
  for (const auto& [id, space] : snapshot.spaces)
    if (model.AddSpace(id, space.profile) != Status::kOk)
      return Invalid("Space identity/Profile");
  for (const auto& [id, window] : snapshot.windows)
    if (model.AddWindow(id) != Status::kOk)
      return Invalid("window identity");
  std::set<Attachment> attachments;
  for (const auto& attachment : snapshot.attachments) {
    if (!attachments.insert(attachment).second ||
        model.Attach(attachment) != Status::kOk)
      return Invalid("attachment duplicate/reference");
  }
  for (const auto& [id, window] : snapshot.windows)
    if (window.selected_space &&
        model.SelectSpace(id, *window.selected_space) != Status::kOk)
      return Invalid("selected Space not attached");
  for (const auto& [id, item] : snapshot.items)
    if (model.AddItem(id, item.owner, item.saved_destination) != Status::kOk)
      return Invalid("saved item identity/owner/destination");
  for (const auto& [id, shortcut] : snapshot.shortcuts)
    if (model.AddShortcut(id, shortcut.profile, shortcut.target) != Status::kOk)
      return Invalid("shortcut identity/target/Profile");
  for (const auto& page : snapshot.pages) {
    // A native report is required by RestoreSnapshot, never inferred from a
    // saved destination, URL equality, tab index, or another Profile's record.
    const std::string destination =
        navigation ? navigation->at(page.id).committed_destination
                   : "about:blank";
    if (const auto* temporary = std::get_if<TemporaryTab>(&page.owner)) {
      if (model.OpenTemporary(page.id, temporary->id, temporary->attachment,
                              destination) != Status::kOk)
        return Invalid("temporary page identity/tab/attachment");
    } else {
      const auto& saved = std::get<SavedPage>(page.owner);
      auto opened = model.OpenItem(saved.item, saved.attachment, page.id);
      const auto* result = std::get_if<OpenedPage>(&opened);
      if (!result || result->disposition != OpenDisposition::kCreated)
        return Invalid("saved page identity/item uniqueness/attachment");
      if (!(model.pages().at(page.id).attachment() == saved.attachment))
        return Invalid("saved page must remain in owning Space");
      if (navigation &&
          model.RecordNavigation(page.id, destination) != Status::kOk)
        return Invalid("restored navigation");
    }
    if (model.pages().at(page.id).profile != page.profile)
      return Invalid("immutable page Profile mismatch");
  }
  std::set<SplitId> splits;
  for (const auto& [id, split] : snapshot.splits)
    if (!splits.insert(id).second ||
        model.SetSplit(id, split.attachment, split.pages) != Status::kOk)
      return Invalid("split identity/membership/attachment");
  return model;
}

// IEEE CRC-32, reflected polynomial, initial/final complement. Detects
// accidental corruption, not adversarial edits. See RFC 1952 section 8.
std::uint32_t Checksum(std::string_view bytes) {
  std::uint32_t crc = 0xffffffffu;
  for (unsigned char byte : bytes) {
    crc ^= byte;
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0);
  }
  return ~crc;
}

class Writer {
 public:
  template <typename T>
  void Integer(T value) {
    static_assert(std::is_unsigned_v<T>);
    std::uint64_t remaining = value;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      bytes.push_back(static_cast<char>(remaining & 0xff));
      remaining >>= 8;
    }
  }
  void Text(std::string_view value) {
    Integer<std::uint64_t>(value.size());
    bytes.append(value);
  }
  template <typename Tag>
  void Identity(const Id<Tag>& id) {
    Text(id.value);
  }
  void Attached(const Attachment& attachment) {
    Identity(attachment.window);
    Identity(attachment.space);
  }
  template <typename T, typename F>
  void Records(const std::vector<T>& records, F write) {
    Integer<std::uint64_t>(records.size());
    for (const auto& record : records)
      write(record);
  }
  std::string bytes;
};

class Reader {
 public:
  explicit Reader(std::string_view bytes) : bytes_(bytes) {}
  template <typename T>
  T Integer() {
    static_assert(std::is_unsigned_v<T>);
    if (bytes_.size() < sizeof(T)) {
      valid_ = false;
      return 0;
    }
    T value = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i)
      value |= static_cast<T>(static_cast<unsigned char>(bytes_[i])) << (8 * i);
    bytes_.remove_prefix(sizeof(T));
    return value;
  }
  std::uint8_t Tag() {
    const auto tag = Integer<std::uint8_t>();
    if (tag > 1)
      valid_ = false;
    return tag;
  }
  std::string Text() {
    const auto length = Integer<std::uint64_t>();
    if (!valid_ || length > bytes_.size()) {
      valid_ = false;
      return {};
    }
    std::string text(bytes_.substr(0, static_cast<std::size_t>(length)));
    bytes_.remove_prefix(static_cast<std::size_t>(length));
    return text;
  }
  template <typename T>
  T Identity() {
    return T(Text());
  }
  Attachment Attached() {
    auto window = Identity<WindowId>();
    auto space = Identity<SpaceId>();
    return {std::move(window), std::move(space)};
  }
  template <typename F>
  auto Records(F read) {
    std::vector<decltype(read())> result;
    const auto count = Integer<std::uint64_t>();
    // Each record consumes at least one byte. This bound follows the remaining
    // input, not a product capacity, and prevents allocation from a bogus
    // count.
    if (!valid_ || count > bytes_.size()) {
      valid_ = false;
      return result;
    }
    for (std::uint64_t i = 0; i < count && valid_; ++i)
      result.push_back(read());
    return result;
  }
  bool complete() const { return valid_ && bytes_.empty(); }

 private:
  std::string_view bytes_;
  bool valid_ = true;
};

DurableSnapshot ProjectModel(const Model& model) {
  DurableSnapshot snapshot;
  for (const auto& [id, profile] : model.profiles())
    snapshot.profiles.push_back(id);
  snapshot.spaces.assign(model.spaces().begin(), model.spaces().end());
  snapshot.windows.assign(model.windows().begin(), model.windows().end());
  snapshot.attachments.assign(model.attachments().begin(),
                              model.attachments().end());
  snapshot.items.assign(model.items().begin(), model.items().end());
  snapshot.shortcuts.assign(model.shortcuts().begin(), model.shortcuts().end());
  for (const auto& [id, page] : model.pages())
    snapshot.pages.push_back({id, page.profile, page.owner});
  snapshot.splits.assign(model.splits().begin(), model.splits().end());
  return snapshot;
}

std::string EncodeRecords(const DurableSnapshot& snapshot);

}  // namespace

Result<DurableSnapshot> CaptureSnapshot(const Model& model,
                                        const SnapshotAdmission& admission) {
  auto snapshot = ProjectModel(model);
  if (auto failure = Admit(snapshot, admission))
    return *failure;
  return snapshot;
}

std::optional<Failure> ValidateSnapshot(const DurableSnapshot& snapshot) {
  auto result = BuildModel(snapshot, nullptr);
  if (const auto* failure = std::get_if<Failure>(&result))
    return *failure;
  return std::nullopt;
}

bool SnapshotMatchesModel(const DurableSnapshot& snapshot, const Model& model) {
  const auto normalized = BuildModel(snapshot, nullptr);
  const auto* graph = std::get_if<Model>(&normalized);
  return graph && EncodeRecords(ProjectModel(*graph)) ==
                      EncodeRecords(ProjectModel(model));
}

Result<EnvelopeVersion> ProbeEnvelope(std::string_view bytes) {
  if (bytes.size() < kHeaderSize || bytes.substr(0, kMagic.size()) != kMagic)
    return Failure{Error::kMalformed, "incomplete/unknown workspace envelope"};
  Reader reader(bytes.substr(kMagic.size(), kHeaderSize - kMagic.size()));
  const auto schema = reader.Integer<std::uint32_t>();
  const auto contract = reader.Integer<std::uint32_t>();
  return EnvelopeVersion{schema, contract};
}

Result<std::string> EncodeSnapshot(const DurableSnapshot& snapshot,
                                   const SnapshotAdmission& admission) {
  if (auto failure = ValidateSnapshot(snapshot))
    return *failure;
  if (auto failure = Admit(snapshot, admission))
    return *failure;
  return EncodeRecords(snapshot);
}

namespace {

std::string EncodeRecords(const DurableSnapshot& snapshot) {
  Writer w;
  w.bytes = kMagic;
  w.Integer(snapshot.schema_version);
  w.Integer(snapshot.contract_version);
  w.Records(snapshot.profiles, [&](const auto& id) { w.Identity(id); });
  w.Records(snapshot.spaces, [&](const auto& record) {
    w.Identity(record.first);
    w.Identity(record.second.profile);
  });
  w.Records(snapshot.windows, [&](const auto& record) {
    w.Identity(record.first);
    w.Integer<std::uint8_t>(record.second.selected_space.has_value());
    if (record.second.selected_space)
      w.Identity(*record.second.selected_space);
  });
  w.Records(snapshot.attachments,
            [&](const auto& attachment) { w.Attached(attachment); });
  w.Records(snapshot.items, [&](const auto& record) {
    w.Identity(record.first);
    const auto& item = record.second;
    w.Integer<std::uint8_t>(std::holds_alternative<ProfileId>(item.owner));
    std::visit([&](const auto& id) { w.Identity(id); }, item.owner);
    w.Text(item.saved_destination);
  });
  w.Records(snapshot.shortcuts, [&](const auto& record) {
    w.Identity(record.first);
    w.Identity(record.second.profile);
    w.Identity(record.second.target);
  });
  w.Records(snapshot.pages, [&](const PageRecord& page) {
    w.Identity(page.id);
    w.Identity(page.profile);
    w.Integer<std::uint8_t>(std::holds_alternative<SavedPage>(page.owner));
    std::visit(
        [&](const auto& owner) {
          using T = std::decay_t<decltype(owner)>;
          if constexpr (std::is_same_v<T, TemporaryTab>)
            w.Identity(owner.id);
          else
            w.Identity(owner.item);
          w.Attached(owner.attachment);
        },
        page.owner);
  });
  w.Records(snapshot.splits, [&](const auto& record) {
    w.Identity(record.first);
    w.Attached(record.second.attachment);
    w.Records(record.second.pages, [&](const auto& id) { w.Identity(id); });
  });
  w.Integer(Checksum(w.bytes));
  return std::move(w.bytes);
}

}  // namespace

Result<DurableSnapshot> DecodeSnapshot(std::string_view bytes) {
  auto envelope = ProbeEnvelope(bytes);
  if (const auto* failure = std::get_if<Failure>(&envelope))
    return *failure;
  const auto version = std::get<EnvelopeVersion>(envelope);
  if (version.schema_version != kSchemaVersion)
    return Failure{Error::kUnsupportedSchema, "unsupported workspace schema"};
  if (version.contract_version != kContractVersion)
    return Failure{Error::kUnsupportedContract,
                   "unsupported lifecycle contract"};
  if (bytes.size() < kHeaderSize + kChecksumSize)
    return Failure{Error::kMalformed, "missing workspace checksum"};
  Reader checksum(bytes.substr(bytes.size() - kChecksumSize));
  const auto payload = bytes.substr(0, bytes.size() - kChecksumSize);
  if (checksum.Integer<std::uint32_t>() != Checksum(payload))
    return Failure{Error::kMalformed, "workspace checksum mismatch"};
  Reader r(payload.substr(kHeaderSize));
  DurableSnapshot s;
  s.profiles = r.Records([&] { return r.Identity<ProfileId>(); });
  s.spaces = r.Records([&] {
    auto id = r.Identity<SpaceId>();
    auto profile = r.Identity<ProfileId>();
    return std::make_pair(std::move(id), Space{std::move(profile)});
  });
  s.windows = r.Records([&] {
    auto id = r.Identity<WindowId>();
    Window window;
    if (r.Tag())
      window.selected_space = r.Identity<SpaceId>();
    return std::make_pair(std::move(id), std::move(window));
  });
  s.attachments = r.Records([&] { return r.Attached(); });
  s.items = r.Records([&] {
    auto id = r.Identity<ItemId>();
    const auto tag = r.Tag();
    ItemOwner owner = tag ? ItemOwner(r.Identity<ProfileId>())
                          : ItemOwner(r.Identity<SpaceId>());
    auto destination = r.Text();
    return std::make_pair(
        std::move(id),
        PersistentItem{std::move(owner), std::move(destination)});
  });
  s.shortcuts = r.Records([&] {
    auto id = r.Identity<ShortcutId>();
    auto profile = r.Identity<ProfileId>();
    auto target = r.Identity<ItemId>();
    return std::make_pair(std::move(id),
                          Shortcut{std::move(profile), std::move(target)});
  });
  s.pages = r.Records([&] {
    auto id = r.Identity<PageId>();
    auto profile = r.Identity<ProfileId>();
    const auto tag = r.Tag();
    auto owner_id = r.Text();
    auto attachment = r.Attached();
    PageOwner owner =
        tag ? PageOwner(
                  SavedPage{ItemId(std::move(owner_id)), std::move(attachment)})
            : PageOwner(TemporaryTab{TemporaryTabId(std::move(owner_id)),
                                     std::move(attachment)});
    return PageRecord{std::move(id), std::move(profile), std::move(owner)};
  });
  s.splits = r.Records([&] {
    auto id = r.Identity<SplitId>();
    auto attachment = r.Attached();
    auto pages = r.Records([&] { return r.Identity<PageId>(); });
    return std::make_pair(std::move(id),
                          Split{std::move(attachment), std::move(pages)});
  });
  if (!r.complete())
    return Failure{Error::kMalformed,
                   "invalid/truncated/trailing workspace data"};
  if (auto failure = ValidateSnapshot(s))
    return *failure;
  return s;
}

Result<Model> RestoreSnapshot(const DurableSnapshot& snapshot,
                              const std::vector<RestoredNavigation>& navigation,
                              const SnapshotAdmission& admission) {
  if (auto failure = ValidateSnapshot(snapshot))
    return *failure;
  if (auto failure = Admit(snapshot, admission))
    return *failure;
  std::map<PageId, RestoredNavigation> reports;
  for (const auto& report : navigation)
    if (report.committed_destination.empty() ||
        !reports.emplace(report.page, report).second)
      return Failure{Error::kNavigationEvidenceRequired,
                     "duplicate/empty native navigation report"};
  if (reports.size() != snapshot.pages.size())
    return Failure{Error::kNavigationEvidenceRequired,
                   "native navigation report set differs from snapshot"};
  for (const auto& page : snapshot.pages) {
    const auto report = reports.find(page.id);
    if (report == reports.end() || report->second.profile != page.profile)
      return Failure{Error::kNavigationEvidenceRequired,
                     "native page identity/Profile report mismatch"};
  }
  return BuildModel(snapshot, &reports);
}

Result<Recovery> Recover(const StoreImage& image) {
  if (!image.original)
    return Failure{Error::kRecoveryRequired,
                   "no committed workspace; candidate cannot prove commit"};
  auto decoded = DecodeSnapshot(*image.original);
  if (const auto* failure = std::get_if<Failure>(&decoded))
    return *failure;
  Recovery recovery{std::get<DurableSnapshot>(std::move(decoded)),
                    {RecoveryNotice::kNavigationAndRendererMemoryNotRestored}};
  if (image.candidate)
    recovery.notices.push_back(RecoveryNotice::kInterruptedCandidateRetained);
  return recovery;
}

Result<PreparedWrite> PrepareWrite(StoreImage source,
                                   const DurableSnapshot& snapshot,
                                   const SnapshotAdmission& admission) {
  if (source.candidate)
    return Failure{
        Error::kRecoveryRequired,
        "interrupted candidate retained; explicit recovery required"};
  if (source.original) {
    const auto original = DecodeSnapshot(*source.original);
    if (const auto* failure = std::get_if<Failure>(&original))
      return *failure;
  }
  auto encoded = EncodeSnapshot(snapshot, admission);
  if (const auto* failure = std::get_if<Failure>(&encoded))
    return *failure;
  StoreImage staged{source.original, std::get<std::string>(std::move(encoded))};
  return PreparedWrite(std::move(source), std::move(staged));
}

Result<StoreImage> CommitPreparedWrite(const StoreImage& current_staged,
                                       const PreparedWrite& prepared,
                                       const SnapshotAdmission& admission) {
  if (!(current_staged == prepared.staged_image()))
    return Failure{Error::kStaleSource,
                   "store changed since preparation; retain every image"};
  const auto decoded = DecodeSnapshot(*prepared.staged_image().candidate);
  if (const auto* failure = std::get_if<Failure>(&decoded))
    return *failure;
  if (auto failure = Admit(std::get<DurableSnapshot>(decoded), admission))
    return *failure;
  return StoreImage{prepared.staged_image().candidate, std::nullopt};
}

}  // namespace arcified::durability
