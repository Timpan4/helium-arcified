// Retrieve only explicitly named, pinned Gitiles files or shallow tree listings.
// Source capture, not a checkout, patch application, or native proof.
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { createHash } from 'node:crypto';
import { mkdir, writeFile } from 'node:fs/promises';
import { dirname } from 'node:path';

const run = promisify(execFile);
const commit = 'a49e2e3d4addfd84e765b52afe230bedb81096db';
const destination = new URL('./chromium/', import.meta.url);
const outcomes = await Promise.allSettled(process.argv.slice(2).map(async (path) => {
  const tree = path.endsWith('/');
  const tag = path === '@tag';
  const url = `https://chromium.googlesource.com/chromium/src/+/${tag ? 'refs/tags/155.0.8059.79' : commit + '/' + path}?format=${tree || tag ? 'JSON' : 'TEXT'}`;
  let stdout;
  try {
    ({ stdout } = await run('curl', ['--fail', '--silent', '--show-error', url]));
  } catch (error) {
    const failure = { path, url, retrievedAt: new Date().toISOString(), error: error.stderr || String(error) };
    const target = new URL('../failures/' + path.replaceAll('/', '__') + '.json', destination);
    await mkdir(dirname(target.pathname), { recursive: true });
    await writeFile(target, JSON.stringify(failure, null, 2) + '\n');
    throw error;
  }
  const bytes = tree || tag ? Buffer.from(stdout.replace(/^\)\]\}'\n/, '')) : Buffer.from(stdout.trim(), 'base64');
  const relative = tag ? 'tag.json' : tree ? path + '_tree.json' : path;
  const target = new URL(relative, destination);
  await mkdir(dirname(target.pathname), { recursive: true });
  await writeFile(target, bytes);
  const metadata = {
    path, commit, url, retrievedAt: new Date().toISOString(),
    kind: tag ? 'tag' : tree ? 'tree' : 'file', bytes: bytes.length,
    sha256: createHash('sha256').update(bytes).digest('hex'),
    ...(tree || tag ? {} : {
      gitBlob: createHash('sha1').update(`blob ${bytes.length}\0`).update(bytes).digest('hex'),
      lines: bytes.toString('utf8').split('\n').length - (bytes.at(-1) === 10 ? 1 : 0),
    }),
    proof: 'Read-only source retrieval only. No prepared tree, compilation, or runtime execution.',
  };
  await writeFile(new URL(relative + '.retrieval.json', destination), JSON.stringify(metadata, null, 2) + '\n');
  return metadata;
}));
for (const outcome of outcomes) {
  if (outcome.status === 'fulfilled') {
    const { path, bytes, kind } = outcome.value;
    console.log(JSON.stringify({ path, bytes, kind }));
  }
  else { console.error(String(outcome.reason)); process.exitCode = 1; }
}
