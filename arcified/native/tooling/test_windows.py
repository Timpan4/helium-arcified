"""Windows plan contracts using small Git/patch fixtures. No Windows runtime claims."""

# Test names describe contracts; fixtures are deliberately synthetic.
# pylint: disable=missing-function-docstring,consider-using-with

import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import windows

PATCH = '''--- a/example.txt
+++ b/example.txt
@@ -1 +1 @@
-before
+after
'''


class WindowsToolingTests(unittest.TestCase):
    """Prove local identities, plan order, no execution, and fail-closed inputs."""

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='arcified windows ')
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.fork = self.directory / 'fork'
        self.platform = self.directory / 'platform'
        self.work = self.directory / 'new work'
        for tree in (self.fork, self.platform):
            (tree / 'patches').mkdir(parents=True)
            (tree / 'patches/series').write_text('change.patch\n', encoding='utf-8')
            (tree / 'patches/change.patch').write_text(PATCH, encoding='utf-8')
            self.git(tree, 'init', '-q')
        for name in ('version.txt', 'revision.txt', 'downloads.ini', 'deps.ini', 'pruning.list',
                     'domain_regex.list', 'domain_substitution.list'):
            (self.fork / name).write_text('fixture\n', encoding='utf-8')
        for name in ('utils', 'devutils', 'resources', 'i18n', 'arcified/native/tooling'):
            path = self.fork / name / 'fixture.txt'
            path.parent.mkdir(parents=True)
            path.write_text('fixture\n', encoding='utf-8')
        (self.fork / 'chromium_version.txt').write_text('155.0.8059.79\n', encoding='utf-8')
        (self.fork / 'flags.gn').write_text('chrome_pgo_phase=0', encoding='utf-8')
        for name in ('build.py', 'downloads.ini', 'downloads.x64.ini', 'revision.txt'):
            (self.platform / name).write_text('fixture\n', encoding='utf-8')
        (self.platform / 'resources').mkdir()
        (self.platform / 'resources/generate_resources.txt').write_text('', encoding='utf-8')
        (self.platform / 'flags.windows.gn').write_text('chrome_pgo_phase=2\ntarget_cpu="x64"',
                                                        encoding='utf-8')
        (self.platform / 'helium-chromium').mkdir()
        self.git(self.platform, 'update-index', '--add', '--cacheinfo',
                 f'160000,{windows.HELIUM_REVISION},helium-chromium')
        for tree in (self.fork, self.platform):
            self.git(tree, 'add', '.')
            self.git(tree, '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                     '-c', 'commit.gpgsign=false', 'commit', '-qm', 'Synthetic input fixture')
        for name, value in [('ROOT', self.fork),
                            ('WINDOWS_REVISION', self.git(self.platform, 'rev-parse', 'HEAD'))]:
            patcher = mock.patch.object(windows, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    @staticmethod
    def git(tree, *args):
        return subprocess.check_output(['git', '-C', str(tree), *args],
                                       text=True,
                                       stderr=subprocess.PIPE).strip()

    def test_default_preflight_uses_fork_identity_without_claiming_windows(self):
        with mock.patch.object(windows.platform, 'system', return_value='Linux'):
            report = windows.preflight()
        self.assertEqual(report['fork']['commit'], self.git(self.fork, 'rev-parse', 'HEAD'))
        self.assertEqual(report['fork']['files_sha256']['flags.gn'],
                         hashlib.sha256(b'chrome_pgo_phase=0').hexdigest())
        self.assertEqual(report['fork']['patches_in_order'][0]['path'], 'change.patch')
        self.assertNotIn('windows', report)
        self.assertFalse(report['toolchain']['native_windows_x64_host'])
        self.assertEqual(report['toolchain']['status'], 'UNVERIFIED')
        for name in ('source_preparation', 'native_build', 'native_launch'):
            self.assertEqual(report[name], 'OPEN')

    def test_working_inputs_are_identified_beyond_commit(self):
        first = windows.preflight()['fork']
        (self.fork / 'flags.gn').write_text('chrome_pgo_phase=1', encoding='utf-8')
        (self.fork / 'resources/new.txt').write_text('untracked', encoding='utf-8')
        second = windows.preflight()['fork']
        self.assertEqual(first['commit'], second['commit'])
        self.assertTrue(second['tracked_changes'])
        self.assertNotEqual(first['files_sha256']['flags.gn'], second['files_sha256']['flags.gn'])
        self.assertIn(str(Path('resources/new.txt')), second['files_sha256'])

    def test_clean_pinned_platform_and_source_markers(self):
        source = self.directory / 'source'
        (source / 'chrome').mkdir(parents=True)
        version = 'MAJOR=155\nMINOR=0\nBUILD=8059\nPATCH=79\n'
        (source / 'chrome/VERSION').write_text(version, encoding='utf-8')
        (source / 'DEPS').write_text('fixture', encoding='utf-8')
        report = windows.preflight(self.platform, source)
        self.assertEqual(report['source']['version'], '155.0.8059.79')
        self.assertIn('does not prove', report['source']['scope'])
        self.assertEqual(report['windows']['upstream_helium_commit'], windows.HELIUM_REVISION)
        (source / 'chrome/VERSION').write_text(version + 'HELIUM_MAJOR=0\n', encoding='utf-8')
        with self.assertRaises(ValueError):
            windows.preflight(self.platform, source)

    def test_engine_revision_and_gitlink_mismatches_fail(self):
        for name in ('WINDOWS_REVISION', 'HELIUM_REVISION'):
            with self.subTest(name=name), mock.patch.object(windows, name, '0' * 40):
                with self.assertRaises(ValueError):
                    windows.preflight(self.platform)
        (self.fork / 'chromium_version.txt').write_text('156.0.8078.47', encoding='utf-8')
        with self.assertRaises(ValueError):
            windows.preflight()

    def test_dirty_platform_and_bad_patch_fail_without_creating_work(self):
        for path, content in [('build.py', 'changed'), ('untracked.txt', 'extra'),
                              ('patches/series', 'missing.patch\n')]:
            with self.subTest(path=path):
                target = self.platform / path
                original = target.read_bytes() if target.exists() else None
                target.write_text(content, encoding='utf-8')
                with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                    windows.prepare(self.platform, self.work)
                self.assertFalse(self.work.exists())
                if original is None:
                    target.unlink()
                else:
                    target.write_bytes(original)

    def test_plan_is_read_only_and_leaves_all_native_acceptance_open(self):
        (self.fork / 'flags.gn').write_text('google_api_key="fixture-secret"', encoding='utf-8')
        with mock.patch.dict(os.environ, {
                'WINSPARKLE_ED_KEY': 'fixture-secret',
                'SISO_REAPI_ADDRESS': 'fixture-secret'
        }):
            report = windows.prepare(self.platform, self.work)
        self.assertFalse(self.work.exists())
        self.assertEqual(report['preparation'], 'PLANNED')
        self.assertEqual(report['execution'], 'UNIMPLEMENTED_OWNER_DEFERRED')
        self.assertEqual(report['native_build'], 'OPEN')
        self.assertEqual(report['native_launch'], 'OPEN')
        self.assertNotIn('fixture-secret', json.dumps(report))
        self.assertEqual(report['steps'][-1]['name'], 'gn-generate')
        for step in report['steps']:
            self.assertFalse(
                any(
                    Path(arg).name in ('build.py', 'autoninja.py', 'ninja', 'siso.exe',
                                       'chrome.exe', 'helium.exe', 'clone.py')
                    for arg in step.get('argv', [])))

    def test_plan_applies_fork_series_before_windows_with_no_fuzz(self):
        platform_patch = PATCH.replace('-before', '-after').replace('+after', '+last')
        (self.platform / 'patches/change.patch').write_text(platform_patch, encoding='utf-8')
        source = self.work / 'src'
        source.mkdir(parents=True)
        (source / 'example.txt').write_text('before\n', encoding='utf-8')
        plan = windows.preparation_plan(self.platform, self.work)
        step = next(item for item in plan if item['name'] == 'patches')
        argv = step['argv'][:]
        self.assertEqual(argv[-2:], [str(self.fork / 'patches'), str(self.platform / 'patches')])
        self.assertIn('--no-fuzz', argv)
        self.assertEqual(Path(argv[argv.index('--patch-bin') + 1]),
                         source / 'third_party/git/usr/bin/patch.exe')
        # Exercise the real inherited patch utility with Linux GNU patch on synthetic files.
        argv[1] = str(windows.input_identity.ROOT / 'utils/patches.py')
        index = argv.index('--patch-bin')
        del argv[index:index + 2]
        subprocess.run(argv, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual((source / 'example.txt').read_text(), 'last\n')

    def test_downloads_resources_and_flags_have_explicit_input_order(self):
        plan = windows.preparation_plan(self.platform, self.work)
        retrieve = next(item for item in plan if item['name'] == 'retrieve')['argv']
        self.assertEqual(retrieve[retrieve.index('-i') + 1:retrieve.index('-c')], [
            str(self.fork / 'downloads.ini'),
            str(self.platform / 'downloads.x64.ini'),
            str(self.platform / 'downloads.ini'),
            str(self.fork / 'deps.ini')
        ])
        names = [step['name'] for step in plan]
        self.assertLess(names.index('source-version'), names.index('prune'))
        self.assertLess(names.index('replace-windows-resources'),
                        names.index('replace-common-resources'))
        flags = next(step for step in plan if step['name'] == 'gn-arguments')
        expected = ('chrome_pgo_phase=0\nchrome_pgo_phase=2\ntarget_cpu="x64"\n'
                    'is_official_build=true\nenable_winsparkle=false\nuse_remoteexec=false\n')
        self.assertEqual(windows.gn_arguments(self.platform), expected)
        self.assertEqual(flags['sha256'], hashlib.sha256(expected.encode('utf-8')).hexdigest())

    def test_existing_and_overlapping_work_directories_fail(self):
        for work in (self.directory, self.fork, self.fork / 'work', self.platform / 'work'):
            with self.subTest(work=work), self.assertRaises(ValueError):
                windows.prepare(self.platform, work)

    def test_cli_rejects_execution_option(self):
        result = subprocess.run([
            sys.executable, '-B', windows.__file__, 'prepare', '--platform-tree',
            str(self.platform), '--work-dir',
            str(self.work), '--execute'
        ],
                                check=False,
                                capture_output=True,
                                text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('unrecognized arguments: --execute', result.stderr)
        self.assertEqual(result.stdout, '')
        self.assertFalse(self.work.exists())

    def test_cli_does_not_create_import_bytecode_by_default(self):
        script = self.directory / 'cli/arcified/native/tooling/windows.py'
        script.parent.mkdir(parents=True)
        script.write_bytes(Path(windows.__file__).read_bytes())
        (script.parent / 'input_identity.py').write_bytes(
            Path(windows.input_identity.__file__).read_bytes())
        result = subprocess.run([sys.executable, str(script), '--help'],
                                check=False,
                                capture_output=True,
                                text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((script.parent / '__pycache__').exists())


if __name__ == '__main__':
    unittest.main()
