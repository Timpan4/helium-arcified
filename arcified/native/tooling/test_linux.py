"""Focused Linux preparation contract tests. No network or Chromium checkout required."""

# Test names describe the contract; addCleanup releases the fixture after each test.
# pylint: disable=missing-function-docstring,consider-using-with

import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import input_identity

SPEC = importlib.util.spec_from_file_location('linux', Path(__file__).with_name('linux.py'))
LINUX = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(LINUX)

PATCH = '''--- a/example.txt
+++ b/example.txt
@@ -1 +1 @@
-before
+after
'''


class LinuxToolingTests(unittest.TestCase):
    """Test observable identity, patch ordering, dry runs, and failure preservation."""

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='arcified linux ')
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)

    def patch_tree(self, name, patch=PATCH):
        """Create a small valid quilt group."""
        tree = self.directory / name
        (tree / 'patches').mkdir(parents=True)
        (tree / 'patches/change.patch').write_text(patch, encoding='utf-8')
        (tree / 'patches/series').write_text('change.patch\n', encoding='utf-8')
        return tree

    def test_patch_identity_tracks_content_and_order(self):
        tree = self.patch_tree('fork')
        first = input_identity.patch_identity(tree)
        changed = PATCH.replace('after', 'changed')
        (tree / 'patches/change.patch').write_text(changed, encoding='utf-8')
        second = input_identity.patch_identity(tree)
        self.assertEqual(first[0]['path'], 'change.patch')
        self.assertNotEqual(first[0]['sha256'], second[0]['sha256'])

    def test_duplicate_missing_and_malformed_patches_fail(self):
        tree = self.patch_tree('fork')
        for series, patch in [('change.patch\nchange.patch\n', PATCH), ('missing.patch\n', PATCH),
                              ('change.patch\n',
                               '--- a/example.txt\n+++ b/example.txt\n@@ -1 +1 @@\n')]:
            with self.subTest(series=series, patch=patch):
                (tree / 'patches/series').write_text(series, encoding='utf-8')
                (tree / 'patches/change.patch').write_text(patch, encoding='utf-8')
                with self.assertRaises(subprocess.CalledProcessError):
                    input_identity.patch_identity(tree)

    def test_source_check_rejects_mismatch_and_prepared_sources(self):
        source = self.directory / 'source'
        (source / 'chrome').mkdir(parents=True)
        version = 'MAJOR=155\nMINOR=0\nBUILD=8059\nPATCH=79\n'
        (source / 'chrome/VERSION').write_text(version, encoding='utf-8')
        (source / 'DEPS').write_text('synthetic DEPS\n', encoding='utf-8')
        identity = input_identity.source_identity(source, '155.0.8059.79')
        self.assertEqual(identity['version'], '155.0.8059.79')
        self.assertIn('does not prove', identity['scope'])
        with self.assertRaises(ValueError):
            input_identity.source_identity(source, '156.0.8078.47')
        (source / 'chrome/VERSION').write_text(version + 'HELIUM_MAJOR=0\n', encoding='utf-8')
        with self.assertRaises(ValueError):
            input_identity.source_identity(source, '155.0.8059.79')

    def test_plan_uses_fork_then_platform_without_compile_or_launch(self):
        fork = self.patch_tree('fork')
        platform_tree = self.patch_tree('platform')
        # Give the platform patch a dependency on the fork patch.
        platform_patch = PATCH.replace('-before', '-after').replace('+after', '+last')
        (platform_tree / 'patches/change.patch').write_text(platform_patch, encoding='utf-8')
        source = self.directory / 'work/src'
        source.mkdir(parents=True)
        (source / 'example.txt').write_text('before\n', encoding='utf-8')
        with mock.patch.object(LINUX, 'ROOT', fork):
            plan = LINUX.preparation_plan(platform_tree, source.parent)
        step = next(item for item in plan if item['name'] == 'patches')
        # Use the actual inherited utility against synthetic files.
        step['argv'][1] = str(LINUX.ROOT / 'utils/patches.py')
        subprocess.run(step['argv'], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual((source / 'example.txt').read_text(), 'last\n')
        self.assertIn('--no-fuzz', step['argv'])
        self.assertEqual(plan[-1]['name'], 'gn-generate')
        for item in plan:
            self.assertFalse(
                any(
                    Path(arg).name in ('autoninja.py', 'ninja', 'siso', 'helium')
                    for arg in item['argv']))

    def test_dry_run_creates_nothing_and_keeps_native_acceptance_open(self):
        work = self.directory / 'work'
        with mock.patch.object(LINUX, 'preflight', return_value={'native_build': 'OPEN'}):
            report = LINUX.prepare(self.directory / 'platform', work, False)
        self.assertFalse(work.exists())
        self.assertEqual(report['preparation'], 'PLANNED')
        self.assertEqual(report['native_build'], 'OPEN')

    def test_gn_arguments_do_not_inherit_host_cache_or_remote_execution(self):
        fork = self.directory / 'fork'
        platform_tree = self.directory / 'platform'
        work = self.directory / 'work'
        fork.mkdir()
        platform_tree.mkdir()
        (work / 'src/out/Default').mkdir(parents=True)
        (fork / 'flags.gn').write_text('chrome_pgo_phase=0', encoding='utf-8')
        (platform_tree / 'flags.linux.gn').write_text('is_official_build=true', encoding='utf-8')
        with mock.patch.object(LINUX, 'ROOT', fork):
            plan = LINUX.preparation_plan(platform_tree, work)
        step = next(item for item in plan if item['name'] == 'gn-arguments')
        subprocess.run(step['argv'], check=True)
        self.assertEqual((work / 'src/out/Default/args.gn').read_text(),
                         'chrome_pgo_phase=0\nis_official_build=true\n'
                         'target_cpu = "x64"\nv8_target_cpu = "x64"\n')

    def test_existing_or_overlapping_work_directory_is_rejected(self):
        platform_tree = self.directory / 'platform'
        for work in [self.directory, LINUX.ROOT / 'build', platform_tree / 'build']:
            with self.subTest(work=work), self.assertRaises(ValueError):
                LINUX.prepare(platform_tree, work, True)

    def test_failed_stage_keeps_logs_and_never_runs_later_stage(self):
        work = self.directory / 'work'
        sentinel = self.directory / 'should not exist'
        failure_command = 'print("fixture failure"); exit(17)'
        steps = [
            {
                'name': 'failure',
                'argv': [sys.executable, '-c', failure_command],
                'cwd': str(self.directory)
            },
            {
                'name': 'later',
                'argv': ['touch', str(sentinel)],
                'cwd': str(self.directory)
            },
        ]
        with mock.patch.object(LINUX, 'preflight', return_value={'native_build': 'OPEN'}), \
             mock.patch.object(LINUX, 'preparation_plan', return_value=steps), \
             mock.patch.object(LINUX.platform, 'system', return_value='Linux'), \
             mock.patch.object(LINUX.platform, 'machine', return_value='x86_64'):
            with self.assertRaises(subprocess.CalledProcessError):
                LINUX.prepare(self.directory / 'platform', work, True)
        self.assertFalse(sentinel.exists())
        report = json.loads((work / 'preparation.json').read_text())
        self.assertEqual(report['preparation'], 'FAILED_OR_INTERRUPTED')
        self.assertEqual(report['steps'][0]['status'], 'FAILED_OR_INTERRUPTED')
        self.assertNotIn('status', report['steps'][1])
        self.assertIn('fixture failure', (work / 'failure.log').read_text())
        self.assertEqual(report['native_build'], 'OPEN')


if __name__ == '__main__':
    unittest.main()
