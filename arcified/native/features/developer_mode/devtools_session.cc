#include "devtools_session.h"

#include <utility>

namespace arcified::developer_mode {
namespace {

Check CompareBinding(const Target& requested, const Target& actual) {
  if (requested.instance.value.empty() || requested.document.value.empty() ||
      requested.binding.value.empty() || actual.instance.value.empty() ||
      actual.document.value.empty() || actual.binding.value.empty()) {
    return Check::kInvalidNativeIdentity;
  }
  if (requested.page.page != actual.page.page)
    return Check::kWrongPage;
  if (requested.page.profile != actual.page.profile)
    return Check::kWrongProfile;
  if (!(requested.page.attachment == actual.page.attachment))
    return Check::kWrongAttachment;
  if (requested.instance != actual.instance)
    return Check::kWrongNativeInstance;
  if (requested.document != actual.document)
    return Check::kWrongDocument;
  if (requested.binding != actual.binding)
    return Check::kWrongBinding;
  return Check::kAccepted;
}

}  // namespace

Check ValidateTarget(const lifecycle::Model& model,
                     const Target& requested,
                     const Observation& observed) {
  switch (commands::RevalidatePageCommandTarget(model, requested.page)) {
    case commands::TargetError::kOk:
      break;
    case commands::TargetError::kInvalidInput:
      return Check::kInvalidPageTarget;
    case commands::TargetError::kPageNotFound:
      return Check::kPageNotFound;
    case commands::TargetError::kAttachmentNotFound:
    case commands::TargetError::kAttachmentMismatch:
      return Check::kWrongAttachment;
    case commands::TargetError::kProfileMismatch:
      return Check::kWrongProfile;
  }
  const auto binding_check = CompareBinding(requested, observed.target);
  if (binding_check != Check::kAccepted)
    return binding_check;
  if (!observed.inspection_allowed)
    return Check::kInspectionDenied;
  if (!observed.tools_visible)
    return Check::kToolsHidden;
  return Check::kAccepted;
}

DevToolsSession::StartResult DevToolsSession::Start(
    const lifecycle::Model& model,
    Target target,
    const Observation& observed,
    DiagnosticSource& source) {
  const auto check = ValidateTarget(model, target, observed);
  if (check != Check::kAccepted)
    return check;
  auto session =
      std::unique_ptr<DevToolsSession>(new DevToolsSession(std::move(target)));
  session->subscription_ = source.Subscribe(session->target_, session->token_);
  if (!session->subscription_)
    return Check::kSubscriptionFailed;
  return session;
}

DevToolsSession::DevToolsSession(Target target)
    : target_(std::move(target)),
      token_(std::make_shared<UpdateToken::Identity>()) {}

DevToolsSession::~DevToolsSession() {
  End(EndReason::kSessionDestroyed);
}

Check DevToolsSession::AcceptUpdate(const lifecycle::Model& model,
                                    const DiagnosticEvent& event,
                                    const Observation& observed) {
  if (cleanup_)
    return Check::kSessionEnded;
  auto check = ValidateTarget(model, target_, observed);
  if (check == Check::kAccepted && !(token_ == event.subscription))
    return Check::kWrongSubscription;
  if (check == Check::kAccepted)
    check = CompareBinding(target_, event.target);
  if (check != Check::kAccepted)
    End(EndReason::kTargetInvalidated);
  return check;
}

Cleanup DevToolsSession::End(EndReason reason) {
  if (cleanup_)
    return *cleanup_;
  cleanup_ = Cleanup::kUnconfirmed;
  auto subscription = std::move(subscription_);
  cleanup_ =
      subscription ? subscription->Cancel(reason) : Cleanup::kConfirmedStopped;
  return *cleanup_;
}

}  // namespace arcified::developer_mode
