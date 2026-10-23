#include "context_admission.h"

#include <map>
#include <set>

namespace arcified::persistence {
namespace {

using lifecycle::ProfileId;
using private_browsing::NativeContextId;

durability::Failure Refused() {
  // No native identities, paths, navigation or private facts in diagnostics.
  return {durability::Error::kPersistenceNotAdmitted,
          "coherent regular native bindings required"};
}

std::optional<durability::Failure> CheckAll(const lifecycle::Model& model,
                                            const NativeContextSource& source) {
  const auto payloads = source.ReadPeerPayloads();
  if (payloads.space_display_and_order != PayloadPresence::kAbsent ||
      payloads.folders != PayloadPresence::kAbsent ||
      payloads.themes != PayloadPresence::kAbsent ||
      payloads.split_presentation != PayloadPresence::kAbsent) {
    return durability::Failure{
        durability::Error::kUnsupportedFeatures,
        "schema 1 peer feature payload absence not established"};
  }

  // Temporary consistency evidence for this invocation, never a live registry.
  std::map<ProfileId, NativeContextId> contexts;
  std::set<NativeContextId> identities;
  for (const auto& [profile, unused] : model.profiles()) {
    const auto observed = source.ReadProfile(profile);
    if (!observed)
      return Refused();
    const auto prepared = profiles::PrepareRegularProfile(
        model, profile, observed->binding, observed->facts);
    const auto* target =
        std::get_if<profiles::RegularProfileServiceTarget>(&prepared);
    if (!target || !identities.insert(target->context).second)
      return Refused();
    contexts.emplace(profile, target->context);
  }

  for (const auto& [window, unused] : model.windows()) {
    const auto observed = source.ReadWindow(window);
    if (!observed || observed->empty())
      return Refused();
    std::set<ProfileId> window_profiles;
    for (const auto& context : *observed) {
      const auto prepared = profiles::PrepareRegularProfile(
          model, context.binding.profile, context.binding, context.facts);
      const auto* target =
          std::get_if<profiles::RegularProfileServiceTarget>(&prepared);
      if (!target || contexts.at(target->profile) != target->context ||
          !window_profiles.insert(target->profile).second) {
        return Refused();
      }
    }
    for (const auto& attachment : model.attachments()) {
      if (attachment.window != window)
        continue;
      const auto& profile = model.spaces().at(attachment.space).profile;
      if (!window_profiles.count(profile))
        return Refused();
    }
  }

  for (const auto& [page, live] : model.pages()) {
    const auto observed = source.ReadPage(page);
    if (!observed || !(observed->attachment == live.attachment()))
      return Refused();
    const auto prepared = profiles::PreparePage(
        model, page, observed->context.binding, observed->context.facts);
    const auto* target = std::get_if<profiles::PagePreparation>(&prepared);
    if (!target ||
        target->space.target.context_class !=
            private_browsing::ContextClass::kRegular ||
        contexts.at(live.profile) != target->space.target.context) {
      return Refused();
    }
  }
  return std::nullopt;
}

}  // namespace

durability::Result<durability::SnapshotAdmission> ComposeSnapshotAdmission(
    const lifecycle::Model& model,
    const NativeContextSource& source) {
  if (const auto refusal = CheckAll(model, source))
    return *refusal;
  return durability::SnapshotAdmission{
      [&model, &source](const lifecycle::ProfileId& profile) {
        return model.profiles().count(profile) && !CheckAll(model, source);
      },
      [&model, &source](const lifecycle::PageId& page,
                        const lifecycle::ProfileId& profile) {
        const auto found = model.pages().find(page);
        return found != model.pages().end() &&
               found->second.profile == profile && !CheckAll(model, source);
      },
      true};
}

}  // namespace arcified::persistence
