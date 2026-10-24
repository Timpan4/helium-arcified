// SPDX-License-Identifier: GPL-3.0-only
// Packet integrity lint only. Inputs are UTF-8 contents supplied by fast_filesystem.
// This neither reads the filesystem nor tests native browser behavior.
function checkPacket(files) {
  const failures = [];
  const byName = new Map(files.map(file => [file.path.split("/").at(-1), file]));
  const json = name => JSON.parse(byName.get(name).content);
  for (const file of files) {
    if (file.error || file.truncated) failures.push("Incomplete input: " + file.path);
    if (!file.content.endsWith("\n") || file.content.includes("\r") ||
        /[ \t]+$/m.test(file.content)) failures.push("Whitespace: " + file.path);
    if (file.path.endsWith(".json") &&
        JSON.stringify(JSON.parse(file.content), null, 2) + "\n" !== file.content) {
      failures.push("JSON formatting: " + file.path);
    }
  }
  const required = [
    "notes", "reader-mode", "paste-go", "recent-tab", "command-education",
    "github-previews", "icloud-sync", "legacy-boost", "ask-page", "arc-sunset",
    "people-cards", "favorite-previews", "dia-memory", "mobile-arc-search",
    "dia-work", "vendor-services"
  ];
  const dispositions = json("dispositions.json");
  const sources = json("sources.json");
  const scenarios = json("verification-scenarios.json");
  const result = json("result.json");
  const native = json("native-source-identities.json");
  const keys = dispositions.records.map(record => record.key);
  if (new Set(keys).size !== keys.length || keys.length !== required.length ||
      required.some(key => !keys.includes(key))) failures.push("Feature coverage");
  const sourceIds = new Set(sources.sources.map(source => source.id));
  if (dispositions.records.some(record =>
      !["implement", "replace", "import", "defer"].includes(record.disposition) ||
      record.implementedByThisPacket !== false || record.nativeAcceptance !== "OPEN" ||
      !record.survivingWorkflow || !record.migration ||
      record.sources.some(id => !sourceIds.has(id)))) failures.push("Disposition/source contract");
  if (scenarios.scenarios.length !== keys.length ||
      keys.some(key => !scenarios.scenarios.some(scenario => scenario.featureKey === key))) {
    failures.push("Scenario coverage");
  }
  if (scenarios.scenarios.some(scenario => scenario.evidence !== null ||
      ["linux", "windows", "macos"].some(os =>
        scenario.platformResults[os]?.nativeAcceptance !== "OPEN" ||
        scenario.platformResults[os]?.nativeExecution !== "NOT_RUN" ||
        scenario.platformResults[os]?.visualExecution !== "NOT_RUN"))) {
    failures.push("Native status honesty");
  }
  if (result.nativeAcceptance !== "OPEN" || result.nativePatch !== null ||
      result.registeredNativeTestChanges.length || result.dependencies.length) {
    failures.push("Research-only result");
  }
  const sync = dispositions.records.find(record => record.key === "icloud-sync");
  if (sync.authority !== "owner_excluded" || sync.disposition !== "defer") {
    failures.push("Settled sync exclusion");
  }
  if (native.files.some(file => file.baseSha256 !== file.resultSha256 || file.modified)) {
    failures.push("Unchanged prepared-source identity");
  }
  const localLinks = [];
  for (const file of files.filter(file => file.path.endsWith(".md"))) {
    const rows = file.content.split("\n");
    let tableWidth = null;
    for (const row of rows) {
      if (row.startsWith("|")) {
        const width = row.split("|").length;
        if (tableWidth === null) tableWidth = width;
        else if (width !== tableWidth) failures.push("Table columns: " + file.path);
      } else {
        tableWidth = null;
      }
    }
    for (const match of file.content.matchAll(/\[[^\]]+\]\(([^)]+)\)/g)) {
      if (!/^https?:/.test(match[1])) {
        const parent = file.path.split("/").slice(0, -1).join("/");
        const parts = [];
        for (const part of (parent + "/" + match[1]).split("/")) {
          if (part === "..") parts.pop();
          else if (part && part !== ".") parts.push(part);
        }
        localLinks.push("/" + parts.join("/"));
      }
    }
  }
  for (const name of result.evidencePaths) {
    if (!byName.has(name)) failures.push("Missing handoff evidence: " + name);
  }
  return {
    failures,
    dispositionRecords: keys.length,
    platformScenarioStates: scenarios.scenarios.length * 3,
    jsonFiles: files.filter(file => file.path.endsWith(".json")).length,
    localLinkTargets: [...new Set(localLinks)]
  };
}
