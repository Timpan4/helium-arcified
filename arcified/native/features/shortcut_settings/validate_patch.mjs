import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {mkdtempSync, mkdirSync, readFileSync, rmSync, writeFileSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {dirname, join, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';

const feature = dirname(fileURLToPath(import.meta.url));
const root = resolve(feature, '../../../..');
const prefix = 'chrome/browser/ui/browser_shortcuts/';
const patch = join(feature, 'native-conflict-preview.patch');
const sourcePath = join(root, 'patches/helium/core/custom-keyboard-shortcuts.patch');
const source = readFileSync(sourcePath);
const identity = createHash('sha1')
    .update(`blob ${source.length}\0`).update(source).digest('hex');
assert.equal(identity, '65b9d7b7f94858720b2311958fcdbd3ab086deeb',
    'Helium source changed. Inspect and rebase the artifact before validating.');
assert.equal(readFileSync(join(root, 'chromium_version.txt'), 'utf8').trim(),
    '155.0.8059.79');

function addedSource(path) {
  const sections = source.toString('utf8').split(/(?=^--- )/m);
  const section = sections.find(value => value.startsWith(
      `--- /dev/null\n+++ b/${path}\n`));
  assert.ok(section, `Missing all-added Helium source: ${path}`);
  const lines = section.trimEnd().split('\n');
  assert.match(lines[2], /^@@ -0,0 \+1,\d+ @@$/);
  const body = lines.slice(3);
  assert.ok(body.every(line => line.startsWith('+')));
  const expectedLines = Number(lines[2].match(/\+1,(\d+)/)[1]);
  assert.equal(body.length, expectedLines);
  return body.map(line => line.slice(1)).join('\n') + '\n';
}

function runGit(args, cwd) {
  const result = spawnSync('git', args, {cwd, encoding: 'utf8'});
  assert.ifError(result.error);
  assert.equal(result.status, 0, `${args.join(' ')}\n${result.stderr}`);
}

const files = [
  'browser_shortcut_service.cc',
  'browser_shortcut_service.h',
  'browser_shortcut_service_preview_unittest.cc',
  'conflict_preview.h',
];
const touched = [...readFileSync(patch, 'utf8').matchAll(/^\+\+\+ b\/(.+)$/gm)]
    .map(match => match[1]);
assert.deepEqual(touched.sort(), files.map(file => prefix + file).sort());

const scratch = mkdtempSync(join(tmpdir(), 'arcified-shortcut-application-'));
try {
  mkdirSync(join(scratch, prefix), {recursive: true});
  for (const file of files.slice(0, 2)) {
    writeFileSync(join(scratch, prefix, file), addedSource(prefix + file));
  }
  runGit(['apply', '--check', '--whitespace=error', patch], scratch);
  runGit(['apply', '--whitespace=error', patch], scratch);
  for (const file of files.slice(2)) {
    assert.equal(readFileSync(join(scratch, prefix, file), 'utf8'),
        readFileSync(join(feature, file), 'utf8'),
        `Patch copy differs from owned source: ${file}`);
  }
  // The artifact only inserts previews. Existing assignment/reset/persistence
  // methods must remain byte-identical, including their validation order.
  const before = addedSource(prefix + 'browser_shortcut_service.cc');
  const after = readFileSync(join(scratch, prefix, 'browser_shortcut_service.cc'), 'utf8');
  const start = after.indexOf('AcceleratorChangePreview BrowserShortcutService::PreviewAcceleratorAssignment(');
  const end = after.indexOf('AssignmentResult BrowserShortcutService::AssignAcceleratorToCommand(', start);
  assert.ok(start !== -1 && end > start);
  assert.equal(after.slice(0, start) + after.slice(end), before);
  runGit(['apply', '--reverse', '--check', '--whitespace=error', patch], scratch);
  console.log(`PASS: pinned Helium ${identity}; forward/reverse patch application; helper/gtest byte identity; original service mutations unchanged.`);
  console.log('OPEN: prepared Chromium tree, GN registration, native compilation/gtest execution, UI integration and installed-platform acceptance.');
} finally {
  rmSync(scratch, {recursive: true, force: true});
}
