#include "preparation.h"

namespace arcified::profiles {
namespace {

std::variant<PreparationRefusal, lifecycle::ProfileId> ResolveAttachment(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment) {
  if (!model.windows().count(attachment.window))
    return PreparationRefusal::kWindowNotFound;
  const auto space = model.spaces().find(attachment.space);
  if (space == model.spaces().end())
    return PreparationRefusal::kSpaceNotFound;
  if (!model.attachments().count(attachment))
    return PreparationRefusal::kNotAttached;
  return space->second.profile;
}

std::variant<PreparationRefusal, ContextTarget> ValidateContext(
    const lifecycle::ProfileId& profile,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts) {
  if (binding.profile != profile)
    return PreparationRefusal::kBindingProfileMismatch;

  const auto classification = private_browsing::ClassifyContext(facts);
  if (classification == private_browsing::ContextClass::kUnknown)
    return PreparationRefusal::kUnknownNativeContext;
  if (binding.context != *facts.context)
    return PreparationRefusal::kNativeContextMismatch;

  switch (classification) {
    case private_browsing::ContextClass::kUnknown:
      return PreparationRefusal::kUnknownNativeContext;
    case private_browsing::ContextClass::kGuest:
    case private_browsing::ContextClass::kUnsupported:
      return PreparationRefusal::kUnsupportedNativeContext;
    case private_browsing::ContextClass::kRegular:
      if (facts.original_profile != profile)
        return PreparationRefusal::kOriginalProfileMismatch;
      break;
    case private_browsing::ContextClass::kOffTheRecord:
      // Original-Profile metadata is deliberately not an equality key. An
      // exact OTR match proves identity only, never ordinary-service access.
      break;
  }
  return ContextTarget{profile, binding.context, classification};
}

}  // namespace

RegularProfileServiceResult PrepareRegularProfile(
    const lifecycle::Model& model,
    const lifecycle::ProfileId& profile,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts) {
  if (!model.profiles().count(profile))
    return PreparationRefusal::kProfileNotFound;
  const auto context = ValidateContext(profile, binding, facts);
  if (const auto* refusal = std::get_if<PreparationRefusal>(&context))
    return *refusal;
  const auto& target = std::get<ContextTarget>(context);
  if (target.context_class != private_browsing::ContextClass::kRegular)
    return PreparationRefusal::kRegularProfileRequired;
  return RegularProfileServiceTarget{target.profile, target.context};
}

SpacePreparationResult PrepareSpace(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts) {
  const auto profile = ResolveAttachment(model, attachment);
  if (const auto* refusal = std::get_if<PreparationRefusal>(&profile))
    return *refusal;
  const auto target =
      ValidateContext(std::get<lifecycle::ProfileId>(profile), binding, facts);
  if (const auto* refusal = std::get_if<PreparationRefusal>(&target))
    return *refusal;
  return SpacePreparation{attachment, std::get<ContextTarget>(target)};
}

PagePreparationResult PreparePage(
    const lifecycle::Model& model,
    const lifecycle::PageId& page_id,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts) {
  const auto page = model.pages().find(page_id);
  if (page == model.pages().end())
    return PreparationRefusal::kPageNotFound;
  // Reuse attachment and context checks, then check the immutable page owner.
  const auto space =
      PrepareSpace(model, page->second.attachment(), binding, facts);
  if (const auto* refusal = std::get_if<PreparationRefusal>(&space))
    return *refusal;
  const auto& prepared = std::get<SpacePreparation>(space);
  if (page->second.profile != prepared.target.profile)
    return PreparationRefusal::kBindingProfileMismatch;
  return PagePreparation{page_id, prepared};
}

RegularProfileServiceResult PrepareRegularProfileServices(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts) {
  const auto profile = ResolveAttachment(model, attachment);
  if (const auto* refusal = std::get_if<PreparationRefusal>(&profile))
    return *refusal;
  return PrepareRegularProfile(model, std::get<lifecycle::ProfileId>(profile),
                               binding, facts);
}

}  // namespace arcified::profiles
