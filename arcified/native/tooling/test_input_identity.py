"""Regressions for shared build-input identity, using retained synthetic inputs."""

# Test method names state their contracts. Fixtures stay available for inspection.
# pylint: disable=missing-function-docstring

import hashlib
import subprocess
import tempfile
import unittest
from pathlib import Path

import input_identity

PATCH = '''--- a/example.txt
+++ b/example.txt
@@ -1 +1 @@
-before
+after
'''


class InputIdentityTests(unittest.TestCase):
    """Exercise the real checker and filesystem traversal without preparing source."""

    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(prefix='arcified-input-identity-'))
        self.tree = self.directory / 'fork'
        (self.tree / 'patches/group').mkdir(parents=True)
        (self.tree / 'resources').mkdir()
        (self.tree / 'patches/group/change.patch').write_text(PATCH, encoding='utf-8')
        (self.tree / 'patches/series').write_text('group/change.patch\n', encoding='utf-8')
        subprocess.run(['git', 'init', '-q', str(self.tree)], check=True)
        subprocess.run([
            'git', '-C',
            str(self.tree), '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
            'commit', '-q', '--allow-empty', '-m', 'Fixture'
        ],
                       check=True)

    def link(self, target, name, directory=False):
        path = self.tree / name
        try:
            path.symlink_to(target, target_is_directory=directory)
        except (NotImplementedError, OSError) as error:
            self.skipTest(f'Host cannot create symlink fixtures: {error}')
        return path

    def test_nested_patch_identity_preserves_series_order(self):
        (self.tree / 'patches/group/first.patch').write_text(PATCH, encoding='utf-8')
        (self.tree / 'patches/series').write_text('group/first.patch\ngroup/change.patch\n',
                                                  encoding='utf-8')
        identity = input_identity.patch_identity(self.tree)
        self.assertEqual([item['path'] for item in identity],
                         ['group/first.patch', 'group/change.patch'])
        self.assertEqual(identity[0]['sha256'], hashlib.sha256(PATCH.encode('utf-8')).hexdigest())

    def test_unused_nested_patch_is_rejected(self):
        (self.tree / 'patches/group/unused.patch').write_text(PATCH, encoding='utf-8')
        with self.assertRaises(subprocess.CalledProcessError):
            input_identity.patch_identity(self.tree)

    def test_file_content_changes_identity(self):
        asset = self.tree / 'resources/asset.txt'
        asset.write_text('before', encoding='utf-8')
        first = input_identity.tree_identity(self.tree, ['resources'])
        asset.write_text('after', encoding='utf-8')
        second = input_identity.tree_identity(self.tree, ['resources'])
        key = str(asset.relative_to(self.tree))
        self.assertEqual(first['files_sha256'][key], hashlib.sha256(b'before').hexdigest())
        self.assertEqual(second['files_sha256'][key], hashlib.sha256(b'after').hexdigest())

    def test_directory_links_are_rejected_including_cycles_and_top_level_inputs(self):
        for name in ('internal', 'cycle'):
            with self.subTest(name=name):
                container = self.tree / 'resources' / name
                container.mkdir()
                target = self.tree / 'patches' if name == 'internal' else container
                link = self.link(target, f'resources/{name}/linked', directory=True)
                with self.assertRaisesRegex(ValueError, 'Directory links'):
                    input_identity.tree_identity(self.tree, [str(link.relative_to(self.tree))])
                with self.assertRaisesRegex(ValueError, 'Directory links'):
                    input_identity.tree_identity(self.tree, [str(container.relative_to(self.tree))])

    def test_cache_filter_keeps_inputs_in_directories_with_pyc_suffix(self):
        resources = self.tree / 'resources'
        (resources / '__pycache__').mkdir()
        (resources / 'assets.pyc').mkdir()
        (resources / '__pycache__/cache.txt').write_text('ignored', encoding='utf-8')
        (resources / 'cache.pyc').write_text('ignored', encoding='utf-8')
        asset = resources / 'assets.pyc/asset.txt'
        asset.write_text('input', encoding='utf-8')
        identity = input_identity.tree_identity(self.tree, ['resources'])
        self.assertEqual(identity['files_sha256'],
                         {str(asset.relative_to(self.tree)): hashlib.sha256(b'input').hexdigest()})

    def test_escaping_directory_and_file_links_are_rejected(self):
        external = self.directory / 'external'
        external.mkdir()
        (external / 'asset.txt').write_text('outside', encoding='utf-8')
        self.link(external, 'resources/linked', directory=True)
        with self.assertRaisesRegex(ValueError, 'escapes its repository'):
            input_identity.tree_identity(self.tree, ['resources'])
        self.link(external / 'asset.txt', 'asset.txt')
        with self.assertRaisesRegex(ValueError, 'escapes its repository'):
            input_identity.tree_identity(self.tree, ['asset.txt'])

    def test_internal_file_link_hashes_effective_content(self):
        asset = self.tree / 'asset.txt'
        asset.write_text('before', encoding='utf-8')
        link = self.link(asset, 'resources/linked.txt')
        first = input_identity.tree_identity(self.tree, ['resources'])
        asset.write_text('after', encoding='utf-8')
        second = input_identity.tree_identity(self.tree, ['resources'])
        key = str(link.relative_to(self.tree))
        self.assertEqual(first['files_sha256'][key], hashlib.sha256(b'before').hexdigest())
        self.assertEqual(second['files_sha256'][key], hashlib.sha256(b'after').hexdigest())

    def test_broken_file_link_is_rejected(self):
        self.link(self.tree / 'missing.txt', 'resources/broken.txt')
        with self.assertRaisesRegex(ValueError, 'Missing build input'):
            input_identity.tree_identity(self.tree, ['resources'])


if __name__ == '__main__':
    unittest.main()
