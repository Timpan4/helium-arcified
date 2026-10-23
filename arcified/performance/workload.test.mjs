// SPDX-License-Identifier: GPL-3.0-only
import assert from "node:assert/strict";
import { test } from "node:test";
import { setImmediate } from "node:timers/promises";

// Tiny contract inputs. These DOM/clock boundaries do not simulate rendering.
async function workload(t, { requests, responseBytes }) {
  const elements = new Map(["status", "echo", "echo-result", "cancel", "export", "state", "identity", "next"]
    .map((id) => [id, Object.assign(new EventTarget(), { value: "", textContent: "" })]));
  const document = Object.assign(new EventTarget(), {
    visibilityState: "visible",
    querySelector: (selector) => {
      assert.ok(elements.has(selector.slice(1)), `Unexpected element ${selector}`);
      return elements.get(selector.slice(1));
    },
    createElement: (tag) => {
      assert.equal(tag, "a");
      return { click() {} };
    },
  });
  const frames = new Map();
  let nextHandle = 0;
  let frameRequested;
  const fetches = [];
  const downloads = [];
  const globals = {
    document,
    window: Object.assign(new EventTarget(), { scrollY: 0 }),
    location: { href: `http://127.0.0.1/workload.html?id=progress&rows=0&textRepeats=0&batchRows=1&requests=${requests}&responseBytes=${responseBytes}` },
    navigator: { userAgent: "isolated-contract-test" },
    innerWidth: 800,
    innerHeight: 600,
    devicePixelRatio: 1,
    scrollY: 0,
    requestAnimationFrame: (callback) => {
      const handle = ++nextHandle;
      frames.set(handle, callback);
      frameRequested?.(handle);
      frameRequested = undefined;
      return handle;
    },
    cancelAnimationFrame: (handle) => frames.delete(handle),
  };
  for (const [name, value] of Object.entries(globals)) {
    const original = Object.getOwnPropertyDescriptor(globalThis, name);
    Object.defineProperty(globalThis, name, { configurable: true, value });
    t.after(() => {
      if (original) Object.defineProperty(globalThis, name, original);
      else delete globalThis[name];
    });
  }
  t.mock.method(globalThis, "fetch", async (url, { signal }) => {
    signal.throwIfAborted();
    fetches.push(url);
    // Exercise production stream draining and byte validation with a real body.
    return new Response("x".repeat(responseBytes));
  });
  t.mock.method(URL, "createObjectURL", (blob) => {
    downloads.push(blob);
    return "blob:isolated-contract-test";
  });
  // A distinct module instance for each isolated case, with no production test API.
  await import(`./workload.mjs?requests=${requests}&bytes=${responseBytes}`);
  return {
    fetches,
    frames,
    waitForFrame: () => frames.size
      ? Promise.resolve(frames.keys().next().value)
      : new Promise((resolve) => { frameRequested = resolve; }),
    advanceFrame: (handle) => {
      const callback = frames.get(handle);
      assert.ok(callback);
      frames.delete(handle);
      callback(performance.now());
    },
    cancel: async () => {
      elements.get("cancel").dispatchEvent(new Event("click"));
      // Let the rejected frame reach build().catch before the next user action.
      await setImmediate();
    },
    exportProgress: async () => {
      elements.get("export").dispatchEvent(new Event("click"));
      assert.equal(downloads.length, 1);
      return JSON.parse(await downloads.shift().text());
    },
  };
}

test("export and cancellation retain consumed work while the post-body frame is pending", async (t) => {
  const page = await workload(t, { requests: 3, responseBytes: 7 });
  const firstFrame = await page.waitForFrame();
  const first = await page.exportProgress();
  assert.equal(first.outcome, "running");
  assert.equal(first.completedRequests, 1);
  assert.equal(first.consumedBytesDecimal, "7");
  assert.equal(page.fetches.length, 1);

  page.advanceFrame(firstFrame);
  await page.waitForFrame();
  const pending = await page.exportProgress();
  assert.equal(pending.outcome, "running");
  assert.equal(pending.completedRequests, 2);
  assert.equal(pending.consumedBytesDecimal, "14");
  for (const name of ["request-start", "response-headers", "request-body-consumed"]) {
    assert.deepEqual(pending.observations.filter((event) => event.name === name).map((event) => event.request), [0, 1]);
  }
  assert.deepEqual(pending.observations.filter((event) => event.name === "request-body-consumed")
    .map((event) => event.responseBytesDecimal), ["7", "7"]);

  await page.cancel();
  const cancelled = await page.exportProgress();
  assert.equal(cancelled.outcome, "cancelled");
  assert.equal(cancelled.completedRequests, 2);
  assert.equal(cancelled.consumedBytesDecimal, "14");
  assert.equal(page.fetches.length, 2, "Cancellation must not start the third request");
  assert.equal(page.frames.size, 0, "Cancellation must remove the pending frame");
  const event = cancelled.observations.find((event) => event.name === "cancelled");
  assert.equal(event.completedRequests, 2);
  assert.equal(event.consumedBytesDecimal, "14");
  assert.ok(!cancelled.observations.some((event) => event.name === "fixture-ready"));
});

test("cancelling the final post-body frame retains zero-byte completion without readiness", async (t) => {
  const page = await workload(t, { requests: 1, responseBytes: 0 });
  await page.waitForFrame();
  const pending = await page.exportProgress();
  assert.equal(pending.outcome, "running");
  assert.equal(pending.completedRequests, 1);
  assert.equal(pending.consumedBytesDecimal, "0");
  await page.cancel();
  const cancelled = await page.exportProgress();
  assert.equal(cancelled.outcome, "cancelled");
  assert.equal(cancelled.completedRequests, 1);
  assert.equal(cancelled.consumedBytesDecimal, "0");
  assert.equal(page.fetches.length, 1);
  assert.equal(page.frames.size, 0);
  assert.ok(!cancelled.observations.some((event) => event.name === "fixture-ready"));
});
