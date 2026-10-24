#ifndef ARCIFIED_NATIVE_FEATURES_CAPTURE_CAPTURE_H_
#define ARCIFIED_NATIVE_FEATURES_CAPTURE_CAPTURE_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::capture {

struct CssSize {
  double width;
  double height;
};
struct PixelSize {
  std::uint32_t width;
  std::uint32_t height;
  friend bool operator==(PixelSize a, PixelSize b) {
    return a.width == b.width && a.height == b.height;
  }
};
// Half-open raster coordinates, never view DIPs or CSS coordinates.
struct PixelRect {
  std::uint32_t x;
  std::uint32_t y;
  std::uint32_t width;
  std::uint32_t height;
};
struct Target {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
  std::optional<lifecycle::SplitId> split;
  std::string document;
  // Host changes this on scroll/layout/zoom, surface replacement or any change
  // that invalidates the capture/picker coordinate mapping. Never reuse it.
  std::string visual_revision;
  friend bool operator==(const Target& a, const Target& b) {
    return a.page == b.page && a.profile == b.profile &&
           a.attachment == b.attachment && a.split == b.split &&
           a.document == b.document && a.visual_revision == b.visual_revision;
  }
};
struct VisibleRegion {
  Target target;
  PixelSize raster_size;
  PixelRect crop;
};

enum class Permission { kUnknown, kAllowed, kDenied };
enum class Provider { kUnavailable, kVisiblePane, kFullPage };
enum class Scope { kVisiblePane, kFullPage };
enum class ExportAction { kCopy, kSave };
struct Policy {
  Permission capture = Permission::kUnknown;
  Permission copy = Permission::kUnknown;
  Permission save = Permission::kUnknown;
  // Unknown/hidden/private/sensitive content must not be attested as safe.
  bool exact_target_content_attested = false;
};
// Fresh authoritative host facts. The model cannot observe native focus,
// document identity, privacy, policy, scroll extents or provider capability.
struct HostFacts {
  Target target;
  CssSize viewport;
  CssSize document;
  Policy policy;
  Provider provider = Provider::kUnavailable;
  bool selected_and_visible = false;
};
struct Request {
  Scope scope;
  double pixels_per_css_pixel;
  std::optional<VisibleRegion> region;
};
struct Plan {
  Target target;
  Request request;
  CssSize css_size;
  PixelSize raster_size;
  PixelSize preview_size;
};
enum class Error {
  kOk,
  kInvalidInput,
  kTargetUnavailable,
  kPolicyRequired,
  kRestricted,
  kProviderUnavailable,
  kFullPageUnavailable,
  kUnrepresentableImage,
  kInvalidRegion,
  kBusy,
  kNoPreview,
  kStaleOperation,
  kStaleSource,
  kInvalidImage,
  kNativeFailed,
  kNativeCancelled,
  kTokenExhausted,
};
using PlanResult = std::variant<Error, Plan>;
PlanResult Prepare(const lifecycle::Model& model,
                   const HostFacts& facts,
                   Request request);

// Tightly packed top-down RGBA8. No untrusted encoded image parser or file
// store. The native provider must isolate the target before supplying bytes.
struct Image {
  PixelSize size;
  std::vector<std::uint8_t> rgba;
};
struct Preview {
  Plan plan;
  Image image;
};
struct Token {
  std::uint64_t value;
  friend bool operator==(Token a, Token b) { return a.value == b.value; }
};
struct CaptureTicket {
  Token token;
  Plan plan;
};
struct ExportTicket {
  Token token;
  ExportAction action;
  std::shared_ptr<const Preview> preview;
};
enum class NativeFailure { kFailed, kCancelled };
enum class NativeExportResult { kSucceeded, kFailed, kCancelled };
using CaptureResult = std::variant<NativeFailure, Image>;

// UI-sequence, transient capture state only. No native provider is installed.
// Tokens are scoped to this noncopyable session and never reused within it.
class Session {
 public:
  Session() = default;
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

  std::variant<Error, CaptureTicket> Begin(const lifecycle::Model& model,
                                           const HostFacts& facts,
                                           Request request);
  Error CompleteCapture(Token token,
                        const lifecycle::Model& model,
                        const HostFacts& fresh_facts,
                        CaptureResult result);
  // Recheck immediately before showing a preview, and on native invalidation
  // events. Failure erases the session's pixels and blocks further exports.
  Error Revalidate(const lifecycle::Model& model, const HostFacts& fresh_facts);
  std::shared_ptr<const Preview> preview() const;
  std::variant<Error, ExportTicket> BeginExport(ExportAction action,
                                                const lifecycle::Model& model,
                                                const HostFacts& fresh_facts);
  // Only call after the native clipboard/file flow reports its actual outcome.
  // Success says nothing about Library insertion. Failed/cancelled exports
  // retain the preview for retry, unless invalidated in the meantime.
  Error CompleteExport(Token token, NativeExportResult result);
  // Capture/preview cancellation clears local state. During native export,
  // first request cancellation through that native flow, then report its
  // result.
  Error Cancel();

 private:
  struct Capturing {
    Token token;
    Plan plan;
  };
  struct Ready {
    std::shared_ptr<const Preview> preview;
  };
  struct Exporting {
    Token token;
    ExportAction action;
    std::shared_ptr<const Preview> preview;
  };
  std::variant<std::monostate, Capturing, Ready, Exporting> state_;
  std::uint64_t generation_ = 0;
};

}  // namespace arcified::capture

#endif  // ARCIFIED_NATIVE_FEATURES_CAPTURE_CAPTURE_H_
