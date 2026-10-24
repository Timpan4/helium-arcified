"""Shared local source, patch, and working-input identity for platform tooling."""

import hashlib
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]


def git(tree, *args):
    """Read Git metadata without exposing remotes or credentials."""
    return subprocess.check_output(['git', '-C', str(tree), *args], text=True).strip()


def digest(path):
    """Hash one build input."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def patch_identity(tree):
    """Reuse the repository's patch checks and preserve its series order."""
    subprocess.run([
        sys.executable, '-B',
        str(ROOT / 'devutils/check_patch_files.py'), '--patches',
        str(tree / 'patches')
    ],
                   check=True,
                   stdout=sys.stderr)
    sys.path.insert(0, str(ROOT / 'utils'))
    try:
        from _common import parse_series # pylint: disable=import-outside-toplevel
        entries = list(parse_series(tree / 'patches/series'))
    finally:
        sys.path.pop(0)
    for entry in entries:
        if not (tree / 'patches' / entry).resolve().is_relative_to(tree / 'patches'):
            raise ValueError(f'Patch escapes its patch group: {entry}')
    return [{'path': entry, 'sha256': digest(tree / 'patches' / entry)} for entry in entries]


def _build_input_files(path, tree):
    """Validate each input before descending, including directory links and junctions."""
    if not path.exists():
        raise ValueError(f'Missing build input: {path}')
    resolved = path.resolve()
    if not resolved.is_relative_to(tree):
        raise ValueError(f'Build input escapes its repository: {path}')
    # Reject directory aliases, including Windows junctions, without traversing cycles.
    # Comparing resolved paths also works on the repository's Python 3.10 baseline.
    if path.is_dir() and (path.is_symlink() or resolved != path.absolute()):
        raise ValueError(f'Directory links are not supported as build inputs: {path}')
    if '__pycache__' in path.parts:
        return
    if path.is_dir():
        for child in sorted(path.iterdir()):
            yield from _build_input_files(child, tree)
    elif path.is_file() and path.suffix != '.pyc':
        yield path


def tree_identity(tree, inputs):
    """Record both commit identity and the actual working files used by preparation."""
    if Path(git(tree, 'rev-parse', '--show-toplevel')).resolve() != tree:
        raise ValueError(f'Expected a repository root: {tree}')
    files = {}
    for name in inputs:
        for item in sorted(_build_input_files(tree / name, tree)):
            files[str(item.relative_to(tree))] = digest(item)
    return {
        'commit': git(tree, 'rev-parse', 'HEAD'),
        'tracked_changes': bool(git(tree, 'status', '--porcelain', '--untracked-files=no')),
        'files_sha256': files,
        'patches_in_order': patch_identity(tree),
    }


def source_identity(tree, expected):
    """Check version markers, without claiming that they prove a pristine source tree."""
    version_file = tree / 'chrome/VERSION'
    parts = dict(line.split('=', 1) for line in version_file.read_text().splitlines() if line)
    version = '.'.join(parts[key] for key in ('MAJOR', 'MINOR', 'BUILD', 'PATCH'))
    if version != expected:
        raise ValueError(f'Chromium source version {version} does not match {expected}')
    if any(key.startswith('HELIUM_') for key in parts):
        raise ValueError('Source already contains Helium versioning; use fresh sources')
    return {
        'version': version,
        'version_sha256': digest(version_file),
        'deps_sha256': digest(tree / 'DEPS'),
        'scope': ('Version markers only. '
                  'This does not prove pristine source or patch applicability.'),
    }


def fork_identity(tree):
    """Identify the fork inputs used by both platform preparation routes."""
    return tree_identity(tree, [
        'chromium_version.txt',
        'version.txt',
        'revision.txt',
        'downloads.ini',
        'deps.ini',
        'flags.gn',
        'pruning.list',
        'domain_regex.list',
        'domain_substitution.list',
        'patches',
        'utils',
        'devutils',
        'resources',
        'i18n',
        'arcified/native/tooling',
    ])
