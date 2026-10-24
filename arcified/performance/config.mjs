// SPDX-License-Identifier: GPL-3.0-only
export function unsignedInteger(value, name, positive = false) {
  if (typeof value !== "string" || !/^\d+$/.test(value)) {
    throw new Error(`${name} must be an explicit unsigned integer`);
  }
  const number = Number(value);
  if (!Number.isSafeInteger(number) || (positive && number === 0)) {
    throw new Error(`${name} must be ${positive ? "positive and " : ""}a safe integer`);
  }
  return number;
}

export function parseWorkload(params) {
  const names = ["id", "rows", "textRepeats", "batchRows", "requests", "responseBytes"];
  for (const name of params.keys()) {
    if (!names.includes(name) || params.getAll(name).length !== 1) {
      throw new Error(`Unknown or duplicate workload choice: ${name}`);
    }
  }
  const id = params.get("id");
  if (!id?.trim()) throw new Error("id must name this synthetic tab");
  return {
    id,
    rows: unsignedInteger(params.get("rows"), "rows"),
    textRepeats: unsignedInteger(params.get("textRepeats"), "textRepeats"),
    batchRows: unsignedInteger(params.get("batchRows"), "batchRows", true),
    requests: unsignedInteger(params.get("requests"), "requests"),
    responseBytes: unsignedInteger(params.get("responseBytes"), "responseBytes"),
  };
}
