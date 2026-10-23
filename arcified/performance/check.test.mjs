// SPDX-License-Identifier: GPL-3.0-only
import assert from "node:assert/strict";
import { once } from "node:events";
import { request } from "node:http";
import { test } from "node:test";
import { parseWorkload } from "./config.mjs";
import { createFixtureServer } from "./server.mjs";

// Values below are tiny contract-test inputs, not recommended benchmark sizes.
test("workloads require each choice and retain explicit zero work", () => {
  const query = "id=alpha&rows=0&textRepeats=0&batchRows=1&requests=0&responseBytes=0";
  assert.deepEqual(parseWorkload(new URLSearchParams(query)), {
    id: "alpha", rows: 0, textRepeats: 0, batchRows: 1, requests: 0, responseBytes: 0,
  });
  for (const invalid of [
    "", query.replace("rows=0&", ""), query.replace("batchRows=1", "batchRows=0"),
    `${query}&rows=1`, `${query}&extra=1`, query.replace("rows=0", "rows=-1"),
    query.replace("rows=0", "rows=1.5"), query.replace("rows=0", "rows=9007199254740992"),
  ]) assert.throws(() => parseWorkload(new URLSearchParams(invalid)));
});

async function fixture(t, cache) {
  const records = [];
  const server = createFixtureServer({ cache, log: (record) => records.push(record) });
  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  t.after(async () => {
    server.closeAllConnections();
    await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  });
  return { server, records, origin: `http://127.0.0.1:${server.address().port}` };
}

test("allowlisted local assets, exact payload, errors and no-store policy", async (t) => {
  const { origin, records } = await fixture(t, "no-store");
  for (const path of ["/", "/index.html", "/workload.html", "/workload.mjs", "/config.mjs", "/style.css"]) {
    const response = await fetch(origin + path);
    assert.equal(response.status, 200);
    assert.equal(response.headers.get("cache-control"), "no-store");
    assert.match(response.headers.get("content-security-policy"), /connect-src 'self'/);
    assert.ok((await response.text()).length > 0);
  }
  const response = await fetch(`${origin}/payload?bytes=7`);
  assert.equal(response.headers.get("content-length"), "7");
  assert.equal(await response.text(), "xxxxxxx");
  assert.equal(await (await fetch(`${origin}/payload?bytes=0`)).text(), "");
  for (const path of ["/payload", "/payload?bytes=-1", "/payload?bytes=1&bytes=2", "/payload?bytes=1&other=2"]) {
    const invalid = await fetch(origin + path);
    assert.equal(invalid.status, 400);
    await invalid.text();
  }
  const head = await fetch(`${origin}/payload?bytes=7`, { method: "HEAD" });
  assert.equal(head.status, 200);
  assert.equal(await head.text(), "");
  const missing = await fetch(`${origin}/server.mjs`);
  assert.equal(missing.status, 404);
  await missing.text();
  const post = await fetch(origin, { method: "POST" });
  assert.equal(post.status, 405);
  await post.text();
  const forbidden = await new Promise((resolve, reject) => {
    const req = request(origin, { headers: { Host: "example.invalid" } }, (res) => {
      res.resume();
      res.on("end", () => resolve(res.statusCode));
    });
    req.on("error", reject);
    req.end();
  });
  assert.equal(forbidden, 403);
  assert.ok(records.some((record) => record.route === "/payload" && record.finished));
});

test("revalidate uses content identity and supports conditional requests", async (t) => {
  const { origin } = await fixture(t, "revalidate");
  const first = await fetch(origin);
  const etag = first.headers.get("etag");
  await first.text();
  assert.ok(etag);
  const cached = await fetch(origin, { headers: { "If-None-Match": etag } });
  assert.equal(cached.status, 304);
  assert.equal(await cached.text(), "");
  assert.equal(cached.headers.get("cache-control"), "public, max-age=0, must-revalidate");
});

test("server does not choose a cache policy for the operator", () => {
  assert.throws(() => createFixtureServer({ cache: undefined }));
});
