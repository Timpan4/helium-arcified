// SPDX-License-Identifier: GPL-3.0-only
// Audits decision evidence only. Does not execute native routing or approve policy.
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";

const read = (name) => readFileSync(new URL(name, import.meta.url), "utf8");
const jsonNames = [
  "authority.json",
  "sources.json",
  "routing-fixtures.json",
  "result.json",
  "compatibility-consolidation.json",
];
const parsed = Object.fromEntries(
  jsonNames.map((name) => {
    const text = read(name);
    const value = JSON.parse(text);
    assert.equal(text, JSON.stringify(value, null, 2) + "\n", name + " format");
    return [name, value];
  }),
);
const fixture = parsed["routing-fixtures.json"];
const result = parsed["result.json"];
const authority = parsed["authority.json"];
assert.equal(authority.issue.key, "focus-routing-contract");
assert.equal(authority.issue.kind, "decision");
assert.equal(authority.sharedContracts.commandIds.primary, 40744);
assert.equal(authority.sharedContracts.commandIds.libraryReserved, 40701);
assert.deepEqual(fixture.commands.page, [
  "address",
  "reload",
  "find",
  "zoom",
  "security",
  "credential",
]);
assert.deepEqual(fixture.commands.workspace, ["move", "keep", "archive"]);
assert.equal(result.nativeAcceptance, "OPEN");
assert.equal(result.issueKey, authority.issue.key);
assert.equal(result.issueUrl, authority.issue.url);
const questions = new Set(result.ownerQuestions.map((q) => q.id));
assert.equal(questions.size, result.ownerQuestions.length);
assert(result.ownerQuestions.every((q) => q.status === "UNRESOLVED"));
const ids = new Set();
const transitions = new Set();
const report = read("decision.md");
for (const scenario of fixture.scenarios) {
  assert(!ids.has(scenario.id), "duplicate fixture " + scenario.id);
  ids.add(scenario.id);
  transitions.add(scenario.transition);
  assert.equal(scenario.commandTargets.page, scenario.activePage);
  assert.equal(scenario.commandTargets.workspace, scenario.selectedObject);
  assert(scenario.visibleCue.length > 0 && scenario.accessibleCue.length > 0);
  assert(["eligible", "blocked", "cancel-stale", "unavailable"].includes(scenario.dispatch));
  if (scenario.activePage === null) {
    assert.equal(scenario.dispatch, "unavailable");
  }
  for (const platform of ["linux", "windows", "macos"]) {
    assert.equal(scenario.nativeResults[platform], "OPEN");
  }
  for (const question of scenario.ownerQuestions) assert(questions.has(question));
  assert(report.includes("| `" + scenario.id + "` |"), "missing table row");
}
for (const transition of [
  "split-enter",
  "split-exit",
  "peek-enter",
  "peek-exit",
  "chrome-enter",
  "chrome-exit",
  "modal-enter",
  "modal-exit",
  "fullscreen-enter",
  "fullscreen-exit",
  "close-success",
  "close-cancel",
  "stale-target",
  "profile-switch",
  "unsupported",
]) {
  assert(transitions.has(transition), "missing transition " + transition);
}
const external = fixture.externalRoutes.scenarios;
const externalIds = new Set();
for (const scenario of external) {
  assert(!externalIds.has(scenario.id), "duplicate external fixture");
  externalIds.add(scenario.id);
  assert.equal(scenario.resolvedTarget, null);
  assert.equal(scenario.availability, "unavailable");
  assert.equal(scenario.policyState, "OPEN");
  assert.notEqual(scenario.knownProfile, scenario.otherLastActiveProfile);
  if (scenario.profileResolution === "unresolved") {
    assert.equal(scenario.knownProfile, null);
  } else {
    assert(scenario.knownProfile);
  }
  for (const platform of ["linux", "windows", "macos"]) {
    assert.equal(scenario.nativeResults[platform], "OPEN");
  }
  for (const question of scenario.ownerQuestions) assert(questions.has(question));
  assert(report.includes("| `" + scenario.id + "` |"), "missing external row");
}
const consolidation = parsed["compatibility-consolidation.json"];
assert.equal(consolidation.nativeAcceptance, "OPEN");
assert.equal(consolidation.compatibilityQuestion.id, "Q2");
assert.equal(
  consolidation.compatibilityCandidate.state,
  "SOURCE_RISK_NOT_RUNTIME_REPRODUCED",
);
for (const id of [
  "os-link-known-profile",
  "os-link-unattributed",
  "notification-page-gone",
  "notification-host-gone",
  "pip-initiator-gone",
  "origin-generation-stale",
  "extension-saveas-no-contents",
  "exact-live-origin",
]) assert(externalIds.has(id), "missing external route " + id);
for (const field of [
  "disposition",
  "evidencePaths",
  "changes",
  "checks",
  "ownerQuestions",
  "dependencies",
  "integrationRequests",
  "remainingCriteria",
]) {
  assert(Object.hasOwn(result, field), "missing result field " + field);
}
for (const entry of parsed["sources.json"].files) {
  assert(/^[0-9a-f]{64}$/.test(entry.sha256), "missing source identity " + entry.path);
}
for (const name of result.evidencePaths) {
  assert(!name.includes("/") && !name.includes("\\"), "evidence must stay in packet");
  const text = read(name);
  assert(text.endsWith("\n"), name + " final newline");
  assert(!/\r/.test(text), name + " LF endings");
  assert(!/[\t ]+$/m.test(text), name + " trailing whitespace");
}
for (const match of report.matchAll(/\]\(([^)]+)\)/g)) {
  const target = match[1];
  if (!target.includes(":") && !target.startsWith("/")) {
    assert(result.evidencePaths.includes(target), "unlisted local reference " + target);
  }
}
console.log(
  "PASS decision artifact audit: " + ids.size +
    " synthetic focus rows; " + externalIds.size +
    " external-route rows; all nine command families; platform results OPEN.",
);
