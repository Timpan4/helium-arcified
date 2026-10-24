# SPDX-License-Identifier: GPL-3.0-only
# Input order: authoritative-issue.json sources.json scenarios.json result.json
.[0] as $authority |
.[1] as $sources |
.[2] as $matrix |
.[3] as $result |
[
  ($authority.issue.key == "runtime-reference"),
  ($authority.issue.url == "https://github.com/Timpan4/helium-arcified/issues/42"),
  ($authority.issue.primaryCommandId == 40799),
  ($authority.baseline == $sources.baseline),
  ($result.issueKey == $authority.issue.key),
  ($result.issueUrl == $authority.issue.url),
  ($result.nativeAcceptance == "OPEN"),
  ($matrix.nativeAcceptance == "OPEN"),
  ($result.nativePatch.status == "INAPPLICABLE_RESEARCH_LEAF"),
  (($matrix.scenarios | map(.id) | sort) ==
    (["peek-pin-url", "tab-handoff", "folder-preview", "split-transitions",
      "sidebar-reveal", "media-return", "native-dialogs"] | sort)),
  (($sources.platforms | map(.id) | sort) ==
    (["arc-macos", "dia-macos", "arc-windows", "dia-windows", "linux"] | sort)),
  ($matrix.scenarios | all(
    (.observations | map(.platformId) | sort) ==
    ($sources.platforms | map(.id) | sort))),
  ($matrix.scenarios | all(
    .sourceIds | all(. as $id | $sources.primarySources | any(.id == $id)))),
  ($matrix.scenarios | all(
    .observations | all(
      .captures == [] and
      (if .platformId == "linux" then .state == "NO_OFFICIAL_REFERENCE_PRODUCT"
       elif (.platformId | endswith("windows")) then .state == "BLOCKED_PACKAGE_AND_RUNTIME"
       else .state == "NOT_RUN" end)))),
  ($sources.platforms | all(
    if (.os == "Windows" or .os == "Linux") then .artifact == null
    else .artifact as $id | $sources.recordedArtifacts | any(.id == $id and .platform == "macOS")
    end)),
  ($sources.recordedArtifacts | all(
    .evidenceClass == "RECORDED_STATIC_INSPECTION" and
    (.sha256 | test("^[a-f0-9]{64}$")))),
  ($sources.localSources | all(.sha256 | test("^[a-f0-9]{64}$"))),
  ($result.remainingCriteria | length == ($authority.issue.accept | length)),
  ($result.remainingCriteria | all(.state == "OPEN"))
] | all
