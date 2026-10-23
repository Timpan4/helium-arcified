// SPDX-License-Identifier: GPL-3.0-only
import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import { createHash } from "node:crypto";
import { Readable } from "node:stream";
import { pipeline } from "node:stream/promises";
import { pathToFileURL } from "node:url";
import { unsignedInteger } from "./config.mjs";

const assets = new Map([
  ["/", ["index.html", "text/html; charset=utf-8"]],
  ["/index.html", ["index.html", "text/html; charset=utf-8"]],
  ["/workload.html", ["workload.html", "text/html; charset=utf-8"]],
  ["/workload.mjs", ["workload.mjs", "text/javascript; charset=utf-8"]],
  ["/config.mjs", ["config.mjs", "text/javascript; charset=utf-8"]],
  ["/style.css", ["style.css", "text/css; charset=utf-8"]],
]);

// The caller chooses cache policy; there is no workload or browser launcher here.
export function createFixtureServer({ cache, log = () => {} }) {
  if (!["no-store", "revalidate"].includes(cache)) {
    throw new Error("Choose cache: no-store or revalidate");
  }
  return createServer(async (req, res) => {
    const started = process.hrtime.bigint();
    let route = "unparsed";
    res.on("close", () => log({
      event: "request-closed",
      route,
      method: req.method,
      status: res.statusCode,
      finished: res.writableFinished,
      startMonotonicNs: started.toString(),
      endMonotonicNs: process.hrtime.bigint().toString(),
    }));
    res.setHeader("Cache-Control", cache === "no-store"
      ? "no-store" : "public, max-age=0, must-revalidate");
    res.setHeader("Content-Security-Policy", "default-src 'none'; script-src 'self'; style-src 'self'; connect-src 'self'; img-src 'self'; form-action 'self'; base-uri 'none'; frame-ancestors 'none'");
    res.setHeader("X-Content-Type-Options", "nosniff");
    try {
      const url = new URL(req.url, "http://127.0.0.1");
      route = url.pathname;
      // Prevent a non-loopback Host from turning this into a rebinding endpoint.
      const host = new URL(`http://${req.headers.host}`).hostname;
      if (host !== "127.0.0.1") {
        res.writeHead(403).end();
        return;
      }
      if (!["GET", "HEAD"].includes(req.method)) {
        res.writeHead(405, { Allow: "GET, HEAD" }).end();
        return;
      }
      if (route === "/payload") {
        for (const key of url.searchParams.keys()) {
          if (key !== "bytes" || url.searchParams.getAll(key).length !== 1) {
            throw new Error("payload requires only bytes");
          }
        }
        const bytes = unsignedInteger(url.searchParams.get("bytes"), "bytes");
        res.writeHead(200, { "Content-Type": "text/plain; charset=utf-8", "Content-Length": bytes });
        if (req.method === "HEAD") {
          res.end();
          return;
        }
        function* body() {
          let remaining = bytes;
          while (remaining > 0) {
            // Use Node's stream buffer size, not a benchmark chunk-size policy.
            const length = Math.min(remaining, res.writableHighWaterMark);
            yield Buffer.alloc(length, "x");
            remaining -= length;
          }
        }
        await pipeline(Readable.from(body()), res);
        return;
      }
      const asset = assets.get(route);
      if (!asset) {
        res.writeHead(404).end();
        return;
      }
      const [filename, type] = asset;
      const content = await readFile(new URL(filename, import.meta.url));
      const etag = `"${createHash("sha256").update(content).digest("hex")}"`;
      res.setHeader("ETag", etag);
      if (cache === "revalidate" && req.headers["if-none-match"] === etag) {
        res.writeHead(304).end();
        return;
      }
      res.writeHead(200, { "Content-Type": type, "Content-Length": content.length });
      res.end(req.method === "HEAD" ? undefined : content);
    } catch (error) {
      if (res.destroyed) return; // Client cancellation already closed the stream.
      if (res.headersSent) {
        res.destroy(error);
        return;
      }
      res.writeHead(400, { "Content-Type": "text/plain; charset=utf-8" }).end(error.message);
    }
  });
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const args = process.argv.slice(2);
  if (args.length !== 4 || args[0] !== "--port" || args[2] !== "--cache") {
    throw new Error("Usage: node server.mjs --port PORT --cache no-store|revalidate");
  }
  const port = unsignedInteger(args[1], "port");
  // TCP's port field is 16 bits; zero asks the OS for an available port.
  if (port > 65535) throw new Error("port exceeds TCP's 16-bit port field");
  const log = (record) => console.log(JSON.stringify(record));
  const server = createFixtureServer({ cache: args[3], log });
  server.on("error", (error) => { console.error(error.message); process.exitCode = 1; });
  server.listen(port, "127.0.0.1", () => log({
    event: "server-start", origin: `http://127.0.0.1:${server.address().port}`,
    cache: args[3], node: process.version, pid: process.pid,
  }));
  for (const signal of ["SIGINT", "SIGTERM"]) {
    process.once(signal, () => {
      log({ event: "server-cancelled", signal });
      server.close();
      server.closeAllConnections();
    });
  }
}
