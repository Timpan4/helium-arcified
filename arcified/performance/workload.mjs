// SPDX-License-Identifier: GPL-3.0-only
import { parseWorkload } from "./config.mjs";

const observations = [];
const controller = new AbortController();
const status = document.querySelector("#status");
let config = null;
let outcome = "configuring";
let echoCount = 0;
let completedRows = 0;
let completedRequests = 0;
let consumedBytes = 0n;

function record(name, detail = {}) {
  observations.push({ name, nowMs: performance.now(), ...detail });
  performance.mark(`fixture:${name}`);
}

function nextFrame(signal) {
  return new Promise((resolve, reject) => {
    signal.throwIfAborted();
    const abort = () => {
      cancelAnimationFrame(handle);
      reject(signal.reason);
    };
    const handle = requestAnimationFrame((timestamp) => {
      signal.removeEventListener("abort", abort);
      resolve(timestamp);
    });
    signal.addEventListener("abort", abort, { once: true });
  });
}

record("script-start", { visibility: document.visibilityState });
for (const type of ["pageshow", "pagehide", "visibilitychange", "focus", "blur", "scroll"]) {
  window.addEventListener(type, (event) => record(type, {
    eventTimestampMs: event.timeStamp,
    persisted: event.persisted ?? null,
    visibility: document.visibilityState,
    scrollY: window.scrollY,
  }), { passive: true });
}
for (const type of ["pointerdown", "keydown", "input"]) {
  document.addEventListener(type, (event) => record("document-input", {
    type, eventTimestampMs: event.timeStamp, trusted: event.isTrusted,
    target: event.target.id || event.target.tagName,
  }), { capture: true, passive: true });
}
performance.addEventListener("resourcetimingbufferfull", () => record("resource-timing-buffer-full"));

document.querySelector("#echo").addEventListener("click", () => {
  echoCount += 1;
  const currentEcho = echoCount;
  document.querySelector("#echo-result").textContent = `Echo ${currentEcho}`;
  record("echo-handler", { echoCount: currentEcho });
  requestAnimationFrame((timestamp) => record("echo-raf-callback", { timestamp, echoCount: currentEcho }));
});
document.querySelector("#cancel").addEventListener("click", () => {
  record("cancel-request", { outcome });
  controller.abort();
});
document.querySelector("#export").addEventListener("click", () => {
  record("export-request");
  const raw = {
    schema: "arcified-page-observations-v1",
    url: location.href, config, outcome,
    timeOriginEpochMs: performance.timeOrigin,
    userAgent: navigator.userAgent,
    viewport: { width: innerWidth, height: innerHeight, devicePixelRatio },
    completedRows, completedRequests, consumedBytesDecimal: consumedBytes.toString(),
    observations,
    entries: ["navigation", "resource", "paint", "mark"].flatMap((type) =>
      performance.getEntriesByType(type).map((entry) => entry.toJSON())),
    state: { syntheticInput: document.querySelector("#state").value, scrollY },
    note: "Page clock and callbacks only. No native timing, physical paint, process memory or quiescence inferred.",
  };
  const url = URL.createObjectURL(new Blob([JSON.stringify(raw, null, 2)], { type: "application/json" }));
  const link = document.createElement("a");
  link.href = url;
  link.download = "page-observations.json";
  link.click();
  // Revoke after navigation has had a task to consume the download URL, without a timer.
  window.addEventListener("pagehide", () => URL.revokeObjectURL(url), { once: true });
});

async function build() {
  config = parseWorkload(new URL(location.href).searchParams);
  outcome = "running";
  document.title = `Synthetic ${config.id}`;
  document.querySelector("#identity").textContent = document.title;
  const next = new URL(location.href);
  next.searchParams.set("id", `${config.id}-next`);
  document.querySelector("#next").href = next.href;
  document.querySelector("#next").hidden = false;
  status.textContent = "Constructing rows and consuming local responses";
  record("construction-start", { config });
  const text = "Public synthetic browser workload. ".repeat(config.textRepeats);
  while (completedRows < config.rows) {
    controller.signal.throwIfAborted();
    const end = Math.min(completedRows + config.batchRows, config.rows);
    const fragment = document.createDocumentFragment();
    while (completedRows < end) {
      const row = document.createElement("li");
      row.textContent = `${config.id} row ${completedRows}: ${text}`;
      fragment.append(row);
      completedRows += 1;
    }
    document.querySelector("#rows").append(fragment);
    await nextFrame(controller.signal);
  }
  record("rows-complete", { completedRows });
  for (let request = 0; request < config.requests; request += 1) {
    controller.signal.throwIfAborted();
    record("request-start", { request });
    const response = await fetch(`/payload?bytes=${config.responseBytes}`, { signal: controller.signal });
    record("response-headers", { request, status: response.status });
    if (!response.ok) throw new Error(`Payload HTTP ${response.status}`);
    // Drain without retaining the response; resource use still includes decoding/stream overhead.
    const reader = response.body.getReader();
    let responseBytes = 0n;
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      consumedBytes += BigInt(value.byteLength);
      responseBytes += BigInt(value.byteLength);
    }
    if (responseBytes !== BigInt(config.responseBytes)) {
      throw new Error("Consumed payload length differs from the chosen workload");
    }
    completedRequests += 1;
    record("request-body-consumed", {
      request, responseBytesDecimal: responseBytes.toString(),
    });
    await nextFrame(controller.signal);
  }
  controller.signal.throwIfAborted();
  outcome = "ready";
  status.textContent = "Fixture ready. Required rows exist and chosen response bodies were consumed.";
  record("fixture-ready", { completedRows, completedRequests, consumedBytesDecimal: consumedBytes.toString() });
}

build().catch((error) => {
  outcome = controller.signal.aborted ? "cancelled" : "error";
  status.textContent = `${outcome}: ${error.message}`;
  record(outcome, { message: error.message, completedRows, completedRequests, consumedBytesDecimal: consumedBytes.toString() });
});
