#!/usr/bin/env python3
"""Identify downstream inputs and prepare Linux sources without compiling Chromium."""

import argparse
import json
import platform
import subprocess
import sys
from pathlib import Path

sys.dont_write_bytecode = True
import input_identity # pylint: disable=wrong-import-position

ROOT = Path(__file__).resolve().parents[3]
# Latest inspected helium-linux revision whose gitlink uses this fork's Chromium version.
LINUX_REVISION = 'b154afccb44729359c816754c7741efb3cb8f497'
HELIUM_REVISION = 'b5145ab1047297970aceb9ca9f48722f6926cc66'
CHROMIUM_VERSION = '155.0.8059.79'


def preflight(platform_tree=None, source_tree=None):
    """Run local identity checks. No downloads, unpacking, GN, or browser build."""
    version = (ROOT / 'chromium_version.txt').read_text().strip()
    report = {
        'chromium_version': version,
        'host': {
            'system': platform.system(),
            'machine': platform.machine(),
            'python': platform.python_version()
        },
        'fork': input_identity.fork_identity(ROOT),
        'native_build': 'OPEN',
        'native_launch': 'OPEN',
    }
    if platform_tree is not None:
        if version != CHROMIUM_VERSION:
            raise ValueError('Review and update the Linux tooling pin for the new Chromium version')
        if input_identity.git(platform_tree, 'rev-parse', 'HEAD') != LINUX_REVISION:
            raise ValueError(f'Linux tooling must be checked out at {LINUX_REVISION}')
        if input_identity.git(platform_tree, 'rev-parse',
                              'HEAD:helium-chromium') != HELIUM_REVISION:
            raise ValueError('Unexpected Linux tooling submodule identity')
        report['linux'] = input_identity.tree_identity(platform_tree, [
            'downloads.ini',
            'flags.linux.gn',
            'revision.txt',
            'patches',
            'scripts',
            'docker',
        ])
        if report['linux']['tracked_changes']:
            raise ValueError('Linux tooling has tracked changes; use a clean pinned checkout')
        report['linux']['upstream_helium_commit'] = HELIUM_REVISION
        report['linux']['patch_source'] = 'Explicit downstream fork, not helium-chromium submodule'
    if source_tree is not None:
        report['source'] = input_identity.source_identity(source_tree, version)
    return report


def preparation_plan(platform_tree, work):
    """Use existing utilities in the order of the pinned Linux build script."""
    source = work / 'src'
    cache = work / 'download_cache'
    steps = []

    def add(name, *argv, cwd=ROOT):
        steps.append({'name': name, 'argv': list(map(str, argv)), 'cwd': str(cwd)})

    def utility(name, script, *argv):
        add(name, sys.executable, ROOT / 'utils' / script, *argv)

    for name, manifests in [('chromium', [ROOT / 'downloads.ini', platform_tree / 'downloads.ini']),
                            ('dependencies', [ROOT / 'deps.ini'])]:
        utility(f'retrieve-{name}', 'downloads.py', 'retrieve', '-i', *manifests, '-c', cache)
        utility(f'unpack-{name}', 'downloads.py', 'unpack', '-i', *manifests, '-c', cache, source)
    add('source-version', sys.executable,
        Path(__file__).resolve(), 'preflight', '--source-tree', source)
    utility('prune', 'prune_binaries.py', '--keep-build-dependencies', source,
            ROOT / 'pruning.list')
    utility('patches', 'patches.py', 'apply', '--no-fuzz', source, ROOT / 'patches',
            platform_tree / 'patches')
    utility('domains', 'domain_substitution.py', 'apply', '-r', ROOT / 'domain_regex.list', '-f',
            ROOT / 'domain_substitution.list', source)
    utility('names', 'name_substitution.py', '--sub', '-t', source, '--backup-path',
            work / 'namesubs.tar')
    utility('translations', 'i18n_apply.py', '-t', source)
    utility('version', 'helium_version.py', '--tree', ROOT, '--platform-tree', platform_tree,
            '--chromium-tree', source)
    add('copy-resources', 'cp', '-a', ROOT / 'resources', work / 'resources')
    resource_list = work / 'resources/generate_resources.txt'
    utility('generate-resources', 'generate_resources.py', resource_list, work / 'resources')
    utility('replace-resources', 'replace_resources.py', work / 'resources/helium_resources.txt',
            work / 'resources', source)
    add('output-directory', 'mkdir', '-p', source / 'out/Default')
    # Match the pinned platform's flags, without host-dependent cache/remote-exec detection.
    add(
        'gn-arguments', 'bash', '-c', 'set -euo pipefail; { cat "$1"; printf "\\n"; cat "$2"; '
        'printf \'\\ntarget_cpu = "x64"\\nv8_target_cpu = "x64"\\n\'; } > "$3"', 'arcified-gn',
        ROOT / 'flags.gn', platform_tree / 'flags.linux.gn', source / 'out/Default/args.gn')
    add('gn-generate',
        source / 'buildtools/linux64/gn',
        'gen',
        'out/Default',
        '--fail-on-unused-args',
        cwd=source)
    return steps


def prepare(platform_tree, work, execute):
    """Plan by default; execute only into a new directory, retaining failure evidence."""
    if work == ROOT or work.is_relative_to(ROOT) or ROOT.is_relative_to(work):
        raise ValueError('Work directory must be separate from the downstream repository')
    if work == platform_tree or work.is_relative_to(platform_tree) or platform_tree.is_relative_to(
            work):
        raise ValueError('Work directory must be separate from the Linux tooling repository')
    if work.exists():
        raise ValueError(
            'Work directory already exists; choose a new directory, do not reuse stamps')
    report = preflight(platform_tree)
    report['steps'] = preparation_plan(platform_tree, work)
    report['preparation'] = 'PLANNED'
    if not execute:
        return report
    if platform.system() != 'Linux' or platform.machine() not in ('x86_64', 'amd64'):
        raise ValueError('The pinned Linux source archive supports this route on Linux x86_64 only')
    work.mkdir(parents=True)
    (work / 'download_cache').mkdir()
    manifest = work / 'preparation.json'

    def save():
        pending = manifest.with_suffix('.json.tmp')
        pending.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        pending.replace(manifest)

    report['preparation'] = 'RUNNING'
    save()
    try:
        for step in report['steps']:
            print(step['name'], file=sys.stderr, flush=True)
            step['status'] = 'RUNNING'
            save()
            with (work / (step['name'] + '.log')).open('wb') as log:
                subprocess.run(step['argv'],
                               cwd=step['cwd'],
                               stdout=log,
                               stderr=subprocess.STDOUT,
                               stdin=subprocess.DEVNULL,
                               check=True)
            step['status'] = 'PASSED'
            save()
    except (OSError, subprocess.CalledProcessError, KeyboardInterrupt):
        step['status'] = 'FAILED_OR_INTERRUPTED'
        report['preparation'] = 'FAILED_OR_INTERRUPTED'
        save()
        raise
    report['args_gn_sha256'] = input_identity.digest(work / 'src/out/Default/args.gn')
    report['preparation'] = 'PREPARED_NOT_BUILT'
    save()
    return report


def main():
    """Command-line boundary."""
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    check = commands.add_parser('preflight', help='Read-only local identity and patch-file checks')
    check.add_argument('--platform-tree', type=Path)
    check.add_argument('--source-tree', type=Path)
    prep = commands.add_parser('prepare', help='Print source preparation plan without compiling')
    prep.add_argument('--platform-tree', type=Path, required=True)
    prep.add_argument('--work-dir', type=Path, required=True)
    prep.add_argument('--execute',
                      action='store_true',
                      help='Retrieve and prepare sources through GN')
    args = parser.parse_args()
    platform_tree = args.platform_tree.resolve() if args.platform_tree is not None else None
    try:
        if args.command == 'preflight':
            source_tree = args.source_tree.resolve() if args.source_tree is not None else None
            result = preflight(platform_tree, source_tree)
        else:
            result = prepare(platform_tree, args.work_dir.resolve(), args.execute)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'ERROR: {error}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
