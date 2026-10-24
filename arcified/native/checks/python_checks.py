"""Run focused native-foundation Python tests or existing scoped style checks."""

import argparse
import os
import platform
import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
TOOLING = ROOT / 'arcified/native/tooling'


def run(*args, env=None):
    """Run a visible command, preserving errors and the repository working directory."""
    print('Running:', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=ROOT, env=env, check=True)


def tests():
    """Discover every component fixture and run the devutils script entrypoint."""
    run('git', '--version')
    run(os.environ.get('PATCH_BIN', 'patch'), '--version')
    suite = unittest.defaultTestLoader.discover(str(TOOLING), pattern='test_*.py')
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    # This existing fixture requires two distinct directories differing only in case.
    # Its own skip is valid on a case-insensitive volume. Other skips, including
    # missing symlink privileges, leave required component coverage incomplete.
    case_sensitive_test = ('test_macos.MacosToolingTests.'
                           'test_distinct_case_sensitive_siblings_allow_new_work')
    unexpected_skips = []
    for test, reason in result.skipped:
        print(f'SKIPPED: {test.id()}: {reason}', flush=True)
        if not (test.id() == case_sensitive_test
                and reason == 'Fixture volume has case-insensitive lookup'):
            unexpected_skips.append(test.id())
    run(sys.executable, '-B', ROOT / 'devutils/tests/test_check_patch_files.py')
    if unexpected_skips:
        print('Incomplete required coverage: ' + ', '.join(unexpected_skips), file=sys.stderr)
    if not result.wasSuccessful() or unexpected_skips or not result.testsRun:
        return 1
    return 0


def lint():
    """Reuse YAPF and the reviewed Pylint import paths and scoped exclusions."""
    native_files = sorted(TOOLING.glob('*.py'))
    runner_files = sorted(Path(__file__).parent.glob('*.py'))
    devutils_files = [
        ROOT / 'devutils/check_patch_files.py', ROOT / 'devutils/tests/test_check_patch_files.py'
    ]
    run(sys.executable, '-B', '-m', 'yapf', '--version')
    run(sys.executable, '-B', '-m', 'pylint', '--version')
    run(sys.executable, '-B', '-m', 'yapf', '--style', ROOT / '.style.yapf', '--diff',
        *native_files, *runner_files, *devutils_files)
    env = os.environ.copy()
    env['PYTHONPATH'] = os.pathsep.join(map(str, [ROOT / 'devutils', ROOT / 'utils', TOOLING]))
    run(sys.executable,
        '-B',
        '-m',
        'pylint',
        '--persistent=n',
        '--disable=duplicate-code',
        *native_files,
        env=env)
    run(sys.executable,
        '-B',
        '-m',
        'pylint',
        '--persistent=n',
        '--disable=wrong-import-position,duplicate-code',
        *devutils_files,
        env=env)
    run(sys.executable, '-B', '-m', 'pylint', '--persistent=n', *runner_files, env=env)
    return 0


def main():
    """CLI entrypoint. No installs, source preparation, or browser jobs are run."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('check', choices=['test', 'lint'])
    args = parser.parse_args()
    print(f'Python: {sys.version}\nExecutable: {sys.executable}', flush=True)
    print(f'Host: {platform.platform()} {platform.machine()}', flush=True)
    for name in ('ImageOS', 'ImageVersion', 'RUNNER_OS', 'RUNNER_ARCH', 'GITHUB_SHA'):
        print(f'{name}: {os.environ.get(name, "unavailable")}', flush=True)
    # T3 can inject launcher overrides that make Python children launch its AppImage.
    # Clear only these overrides for this process and its children.
    os.environ.pop('PYTHONEXECUTABLE', None)
    os.environ.pop('__PYVENV_LAUNCHER__', None)
    return tests() if args.check == 'test' else lint()


if __name__ == '__main__':
    sys.exit(main())
