"""macOS planning contracts with small local fixtures, without macOS runtime claims."""

# Test names describe observable contracts; all sources and Git histories are synthetic.
# pylint: disable=missing-function-docstring,consider-using-with

import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from contextlib import contextmanager
from pathlib import Path
from unittest import mock

import macos

PATCH = '''--- a/example.txt
+++ b/example.txt
@@ -1 +1 @@
-before
+middle
'''


class MacosToolingTests(unittest.TestCase):
    """Prove input identity, explicit architectures, plan order, and read-only failure."""

    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(prefix='arcified-macos-')).resolve()
        self.fork = self.directory / 'fork'
        self.platform = self.directory / 'platform'
        self.work = self.directory / 'new-work'
        for tree in (self.fork, self.platform):
            (tree / 'patches').mkdir(parents=True)
            self.git(tree, 'init', '-q')
        (self.fork / 'patches/series').write_text('first.patch\nsecond.patch\n', encoding='utf-8')
        (self.fork / 'patches/first.patch').write_text(PATCH, encoding='utf-8')
        second_patch = PATCH.replace('-before', '-middle').replace('+middle', '+after')
        (self.fork / 'patches/second.patch').write_text(second_patch, encoding='utf-8')
        (self.platform / 'patches/series').write_text('final.patch\n', encoding='utf-8')
        final_patch = PATCH.replace('-before', '-after').replace('+middle', '+last')
        (self.platform / 'patches/final.patch').write_text(final_patch, encoding='utf-8')
        for name in ('version.txt', 'revision.txt', 'downloads.ini', 'deps.ini', 'pruning.list',
                     'domain_regex.list', 'domain_substitution.list'):
            (self.fork / name).write_text('fixture\n', encoding='utf-8')
        for name in ('utils', 'devutils', 'resources', 'i18n', 'arcified/native/tooling'):
            path = self.fork / name / 'fixture.txt'
            path.parent.mkdir(parents=True)
            path.write_text('fixture\n', encoding='utf-8')
        (self.fork / 'chromium_version.txt').write_text('155.0.8059.79\n', encoding='utf-8')
        (self.fork / 'flags.gn').write_text('chrome_pgo_phase=0', encoding='utf-8')
        for name in ('build.sh', 'env.sh', 'downloads.ini', 'downloads.arm64.ini',
                     'downloads.x64.ini', 'revision.txt', 'docs/building.md',
                     'resources/platform_resources.txt', 'devutils/shared.sh'):
            path = self.platform / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('fixture\n', encoding='utf-8')
        (self.platform / 'flags.macos.gn').write_text('is_official_build=true', encoding='utf-8')
        (self.platform / 'helium-chromium').mkdir()
        self.git(self.platform, 'update-index', '--add', '--cacheinfo',
                 f'160000,{macos.HELIUM_REVISION},helium-chromium')
        for tree in (self.fork, self.platform):
            self.git(tree, 'add', '.')
            self.git(tree, '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                     '-c', 'commit.gpgsign=false', 'commit', '-qm', 'Synthetic input fixture')
        for name, value in [('ROOT', self.fork),
                            ('MACOS_REVISION', self.git(self.platform, 'rev-parse', 'HEAD'))]:
            patcher = mock.patch.object(macos, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    @staticmethod
    def git(tree, *args):
        return subprocess.check_output(['git', '-C', str(tree), *args],
                                       text=True,
                                       stderr=subprocess.PIPE).strip()

    @contextmanager
    def case_insensitive_input_lookup(self):
        # Emulate ordinary alternate-case components, not symlink canonicalization.
        # Keep real stat identities and the production Path.resolve()/prepare() path.
        def lookup(operation, path, *args, **kwargs):
            path = Path(path)
            for tree in (self.fork, self.platform):
                alias = tree.with_name(tree.name.upper())
                if path.is_relative_to(alias):
                    path = tree / path.relative_to(alias)
                    break
            return operation(path, *args, **kwargs)

        stat, lstat = os.stat, os.lstat
        with mock.patch.object(os, 'stat', side_effect=lambda *a, **k: lookup(stat, *a, **k)), \
                mock.patch.object(os, 'lstat', side_effect=lambda *a, **k: lookup(lstat, *a, **k)):
            yield

    def test_preflight_identifies_working_inputs_and_leaves_native_acceptance_open(self):
        with mock.patch.object(macos.platform, 'system', return_value='Linux'):
            report = macos.preflight()
        self.assertEqual(report['fork']['commit'], self.git(self.fork, 'rev-parse', 'HEAD'))
        self.assertEqual(report['fork']['files_sha256']['flags.gn'],
                         hashlib.sha256(b'chrome_pgo_phase=0').hexdigest())
        self.assertEqual([item['path'] for item in report['fork']['patches_in_order']],
                         ['first.patch', 'second.patch'])
        self.assertFalse(report['toolchain']['native_macos_host'])
        self.assertNotIn('macos', report)
        for name in ('native_acceptance', 'source_preparation', 'patch_application',
                     'gn_generation', 'native_build', 'native_launch'):
            self.assertEqual(report[name], 'OPEN')
        with mock.patch.object(macos.platform, 'system',
                               return_value='Darwin'), mock.patch.object(macos.platform,
                                                                         'machine',
                                                                         return_value='arm64'):
            native_report = macos.preflight()
        self.assertTrue(native_report['toolchain']['native_macos_host'])
        self.assertEqual(native_report['toolchain']['status'], 'UNVERIFIED')

    def test_working_changes_and_untracked_fork_inputs_affect_identity(self):
        first = macos.preflight()['fork']
        (self.fork / 'flags.gn').write_text('chrome_pgo_phase=1', encoding='utf-8')
        (self.fork / 'resources/new.txt').write_text('untracked', encoding='utf-8')
        second = macos.preflight()['fork']
        self.assertEqual(first['commit'], second['commit'])
        self.assertTrue(second['tracked_changes'])
        self.assertNotEqual(first['files_sha256']['flags.gn'], second['files_sha256']['flags.gn'])
        self.assertIn(str(Path('resources/new.txt')), second['files_sha256'])

    def test_source_markers_are_checked_without_claiming_provenance(self):
        source = self.directory / 'source'
        (source / 'chrome').mkdir(parents=True)
        version = 'MAJOR=155\nMINOR=0\nBUILD=8059\nPATCH=79\n'
        (source / 'chrome/VERSION').write_text(version, encoding='utf-8')
        (source / 'DEPS').write_text('fixture', encoding='utf-8')
        report = macos.preflight(self.platform, source)
        self.assertEqual(report['source']['version'], '155.0.8059.79')
        self.assertEqual(report['source']['deps_sha256'], hashlib.sha256(b'fixture').hexdigest())
        self.assertIn('does not prove', report['source']['scope'])
        self.assertEqual(report['macos']['upstream_helium_commit'], macos.HELIUM_REVISION)
        for content in (version.replace('PATCH=79', 'PATCH=84'), version + 'HELIUM_MAJOR=0\n'):
            (source / 'chrome/VERSION').write_text(content, encoding='utf-8')
            with self.assertRaises(ValueError):
                macos.preflight(self.platform, source)

    def test_engine_revision_and_gitlink_mismatches_fail(self):
        for name in ('MACOS_REVISION', 'HELIUM_REVISION'):
            with self.subTest(name=name), mock.patch.object(macos, name, '0' * 40):
                with self.assertRaises(ValueError):
                    macos.prepare(self.platform, self.work, 'arm64', 'arm64')
        (self.fork / 'chromium_version.txt').write_text('155.0.8059.84', encoding='utf-8')
        with self.assertRaises(ValueError):
            macos.preflight()
        self.assertFalse(self.work.exists())

    def test_dirty_platform_and_invalid_series_fail_without_creating_work(self):
        for tree, name, content in [(self.platform, 'build.sh', 'changed'),
                                    (self.platform, 'untracked.txt', 'extra'),
                                    (self.platform, 'patches/series', 'missing.patch\n'),
                                    (self.fork, 'patches/series', 'first.patch\nfirst.patch\n')]:
            with self.subTest(tree=tree, name=name):
                path = tree / name
                original = path.read_bytes() if path.exists() else None
                path.write_text(content, encoding='utf-8')
                with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                    macos.prepare(self.platform, self.work, 'arm64', 'arm64')
                self.assertFalse(self.work.exists())
                if original is None:
                    path.unlink()
                else:
                    path.write_bytes(original)

    def test_plan_is_read_only_and_does_not_expose_flags_or_environment_secrets(self):
        (self.fork / 'flags.gn').write_text('google_api_key="fixture-secret"', encoding='utf-8')
        before = {
            str(path): path.read_bytes()
            for tree in (self.fork, self.platform)
            for path in tree.rglob('*') if path.is_file() and '.git' not in path.parts
        }
        with mock.patch.dict(
                os.environ, {
                    'MACOS_CERTIFICATE_NAME': 'fixture-secret',
                    'PROD_MACOS_SPARKLE_ED_PUB_KEY': 'fixture-secret',
                    'SISO_REAPI_ADDRESS': 'fixture-secret',
                }):
            report = macos.prepare(self.platform, self.work, 'arm64', 'x64')
        after = {
            str(path): path.read_bytes()
            for tree in (self.fork, self.platform)
            for path in tree.rglob('*') if path.is_file() and '.git' not in path.parts
        }
        self.assertEqual(before, after)
        self.assertFalse(self.work.exists())
        self.assertEqual(report['preparation'], 'PLANNED')
        self.assertEqual(report['execution'], 'UNIMPLEMENTED_OWNER_DEFERRED')
        self.assertEqual(report['native_acceptance'], 'OPEN')
        self.assertNotIn('fixture-secret', json.dumps(report))
        self.assertTrue(all(step['status'] == 'NOT_RUN' for step in report['steps']))
        self.assertEqual(report['steps'][-1]['name'], 'gn-generate')
        for step in report['steps']:
            self.assertFalse(
                any(
                    Path(arg).name in ('build.sh', 'dev.sh', 'autoninja.py', 'siso', 'ninja',
                                       'xcodebuild', 'codesign', 'Helium', 'clone.py',
                                       'configure_siso.py') for arg in step.get('argv', [])))

    def test_host_archive_and_target_flags_are_independent(self):
        for host in ('arm64', 'x64'):
            for target in ('arm64', 'x64'):
                with self.subTest(host=host, target=target):
                    report = macos.prepare(self.platform, self.work, host, target)
                    self.assertEqual(report['archive_host_architecture'], host)
                    self.assertEqual(report['target'], f'mac-{target}')
                    retrieve = next(step for step in report['steps'] if step['name'] == 'retrieve')
                    argv = retrieve['argv']
                    self.assertEqual(argv[argv.index('-i') + 1:argv.index('-c')], [
                        str(self.fork / 'downloads.ini'),
                        str(self.platform / f'downloads.{host}.ini'),
                        str(self.platform / 'downloads.ini'),
                        str(self.fork / 'deps.ini')
                    ])
                    flags = next(step for step in report['steps'] if step['name'] == 'gn-arguments')
                    expected = ('chrome_pgo_phase=0\nis_official_build=true\n'
                                f'target_cpu = "{target}"\nchrome_pgo_phase=2\n'
                                'enable_sparkle=false\ninclude_branded_entitlements=false\n'
                                'use_remoteexec=false\ncc_wrapper=""\n')
                    self.assertEqual(macos.gn_arguments(self.platform, target), expected)
                    reconstructed = flags['separator'].join(
                        Path(path).read_text(encoding='utf-8') for path in flags['inputs'])
                    self.assertEqual(reconstructed + flags['append'], expected)
                    self.assertEqual(flags['sha256'],
                                     hashlib.sha256(expected.encode('utf-8')).hexdigest())

    def test_plan_applies_all_fork_patches_then_platform_patches_without_fuzz(self):
        source = self.work / 'src'
        source.mkdir(parents=True)
        (source / 'example.txt').write_text('before\n', encoding='utf-8')
        plan = macos.preparation_plan(self.platform, self.work, 'arm64', 'arm64')
        step = next(item for item in plan if item['name'] == 'patches')
        self.assertEqual(
            step['argv'][-2:],
            [str(self.fork / 'patches'), str(self.platform / 'patches')])
        self.assertIn('--no-fuzz', step['argv'])
        argv = step['argv'][:]
        argv[1] = str(macos.input_identity.ROOT / 'utils/patches.py')
        subprocess.run(argv, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual((source / 'example.txt').read_text(), 'last\n')
        names = [item['name'] for item in plan]
        self.assertLess(names.index('source-version'), names.index('prune'))
        self.assertLess(names.index('prune'), names.index('patches'))
        self.assertLess(names.index('patches'), names.index('domains'))

    def test_resources_are_generated_and_replaced_in_the_work_tree(self):
        plan = macos.preparation_plan(self.platform, self.work, 'x64', 'arm64')
        names = [step['name'] for step in plan]
        self.assertLess(names.index('copy-common-resources'),
                        names.index('generate-common-resources'))
        self.assertLess(names.index('replace-macos-resources'),
                        names.index('replace-common-resources'))
        for step in plan:
            if step['name'].startswith(('generate-', 'replace-')):
                self.assertTrue(Path(step['argv'][2]).is_relative_to(self.work))
        prune = next(step for step in plan if step['name'] == 'prune')
        self.assertIn('--keep-contingent-paths', prune['argv'])

    def test_invalid_architectures_existing_and_overlapping_work_paths_fail(self):
        for host, target in [('riscv64', 'arm64'), ('arm64', 'x86_64')]:
            with self.subTest(host=host, target=target), self.assertRaises(ValueError):
                macos.prepare(self.platform, self.work, host, target)
        for work in (self.directory, self.fork, self.fork / 'work', self.platform / 'work',
                     self.directory / 'with spaces'):
            with self.subTest(work=work), self.assertRaises(ValueError):
                macos.prepare(self.platform, work, 'arm64', 'arm64')

    def test_case_insensitive_aliases_into_either_input_reject_new_work(self):
        for tree in (self.fork, self.platform):
            (tree / 'existing').mkdir()
        with self.case_insensitive_input_lookup():
            for tree in (self.fork, self.platform):
                alias = tree.with_name(tree.name.upper())
                self.assertTrue(alias.samefile(tree))
                for suffix in ('new-work', 'existing/new-work', 'missing/parents/new-work'):
                    with self.subTest(tree=tree, suffix=suffix):
                        work = (alias / suffix).resolve()
                        self.assertEqual(work, alias / suffix)
                        with self.assertRaisesRegex(ValueError, 'separate from both input'):
                            macos.prepare(self.platform, work, 'arm64', 'x64')
                        self.assertFalse(work.exists())
                        self.assertFalse((tree / 'missing').exists())

    def test_case_insensitive_lookup_allows_separate_work_without_creating_parents(self):
        work = self.directory / 'separate/missing/new-work'
        with self.case_insensitive_input_lookup():
            report = macos.prepare(self.platform, work.resolve(), 'x64', 'arm64')
        self.assertEqual(report['preparation'], 'PLANNED')
        self.assertEqual(report['execution'], 'UNIMPLEMENTED_OWNER_DEFERRED')
        self.assertTrue(all(step['status'] == 'NOT_RUN' for step in report['steps']))
        for name in ('native_acceptance', 'source_preparation', 'patch_application',
                     'gn_generation', 'native_build', 'native_launch'):
            self.assertEqual(report[name], 'OPEN')
        self.assertFalse((self.directory / 'separate').exists())

    def test_distinct_case_sensitive_siblings_allow_new_work(self):
        for tree in (self.fork, self.platform):
            sibling = tree.with_name(tree.name.upper())
            sibling.mkdir(exist_ok=True)
            if sibling.samefile(tree):
                self.skipTest('Fixture volume has case-insensitive lookup')
            with self.subTest(tree=tree):
                work = sibling / 'missing/new-work'
                report = macos.prepare(self.platform, work.resolve(), 'arm64', 'arm64')
                self.assertEqual(report['preparation'], 'PLANNED')
                self.assertTrue(all(step['status'] == 'NOT_RUN' for step in report['steps']))
                self.assertFalse((sibling / 'missing').exists())

    def test_cli_rejects_execution_option_and_does_not_create_bytecode(self):
        script = self.directory / 'cli/arcified/native/tooling/macos.py'
        script.parent.mkdir(parents=True)
        script.write_bytes(Path(macos.__file__).read_bytes())
        (script.parent / 'input_identity.py').write_bytes(
            Path(macos.input_identity.__file__).read_bytes())
        result = subprocess.run([sys.executable, str(script), '--help'],
                                check=False,
                                capture_output=True,
                                text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((script.parent / '__pycache__').exists())
        result = subprocess.run([
            sys.executable,
            str(script), 'prepare', '--platform-tree',
            str(self.platform), '--work-dir',
            str(self.work), '--host-arch', 'arm64', '--target-arch', 'x64', '--execute'
        ],
                                check=False,
                                capture_output=True,
                                text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('unrecognized arguments: --execute', result.stderr)
        self.assertEqual(result.stdout, '')
        self.assertFalse(self.work.exists())


if __name__ == '__main__':
    unittest.main()
