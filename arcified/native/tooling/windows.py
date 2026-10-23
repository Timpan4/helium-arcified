#!/usr/bin/env python3
"""Print local Windows input identity and a preparation plan. Never execute the plan."""

import argparse
import hashlib
import json
import platform
import subprocess
import sys
from pathlib import Path

sys.dont_write_bytecode = True
import input_identity # pylint: disable=wrong-import-position

ROOT = Path(__file__).resolve().parents[3]
# Newest inspected Windows revision with exactly this fork's Chromium version.
WINDOWS_REVISION = '555c9467373779ea030d6b555d7bd05b48d96d8c'
HELIUM_REVISION = 'b5145ab1047297970aceb9ca9f48722f6926cc66'
CHROMIUM_VERSION = '155.0.8059.79'


def preflight(platform_tree=None, source_tree=None):
    """Read local inputs; host identity is not Windows toolchain verification."""
    version = (ROOT / 'chromium_version.txt').read_text(encoding='utf-8').strip()
    if version != CHROMIUM_VERSION:
        raise ValueError('Review and update the Windows tooling pin for the new Chromium version')
    report = {
        'chromium_version': version,
        'host': {
            'system': platform.system(),
            'machine': platform.machine(),
            'python': platform.python_version(),
        },
        'target': 'win-x64',
        'fork': input_identity.fork_identity(ROOT),
        'toolchain': {
            'status': 'UNVERIFIED',
            'native_windows_x64_host': platform.system() == 'Windows'
            and platform.machine().lower() in ('amd64', 'x86_64'),
            'scope': ('No Visual Studio, SDK, compiler, extractor, filesystem policy, '
                      'or Windows executable was probed.'),
        },
        'source_preparation': 'OPEN',
        'native_build': 'OPEN',
        'native_launch': 'OPEN',
    }
    if platform_tree is not None:
        if input_identity.git(platform_tree, 'rev-parse', 'HEAD') != WINDOWS_REVISION:
            raise ValueError(f'Windows tooling must be checked out at {WINDOWS_REVISION}')
        if input_identity.git(platform_tree, 'rev-parse',
                              'HEAD:helium-chromium') != HELIUM_REVISION:
            raise ValueError('Unexpected Windows tooling submodule identity')
        report['windows'] = input_identity.tree_identity(platform_tree, [
            'build.py',
            'downloads.ini',
            'downloads.x64.ini',
            'flags.windows.gn',
            'revision.txt',
            'patches',
            'resources',
        ])
        if report['windows']['tracked_changes'] or input_identity.git(
                platform_tree, 'ls-files', '--others', '--exclude-standard'):
            raise ValueError('Windows tooling has changes; use a clean pinned checkout')
        report['windows']['upstream_helium_commit'] = HELIUM_REVISION
        report['windows'][
            'patch_source'] = 'Explicit downstream fork, not helium-chromium submodule'
    if source_tree is not None:
        report['source'] = input_identity.source_identity(source_tree, version)
    return report


def gn_arguments(platform_tree):
    """Match pinned production flags with updates disabled and no ambient credentials."""
    return ((ROOT / 'flags.gn').read_text(encoding='utf-8') + '\n' +
            (platform_tree / 'flags.windows.gn').read_text(encoding='utf-8') +
            '\nis_official_build=true\nenable_winsparkle=false\nuse_remoteexec=false\n')


def preparation_plan(platform_tree, work):
    """Describe the pinned x64 tarball route using the fork's utilities and inputs."""
    source = work / 'src'
    cache = work / 'download_cache'
    steps = []

    def action(name, operation, **fields):
        steps.append({'name': name, 'operation': operation, **fields})

    def command(name, script, *argv, cwd=ROOT):
        action(name, 'command', argv=list(map(str, [sys.executable, script, *argv])), cwd=str(cwd))

    def utility(name, script, *argv):
        command(name, ROOT / 'utils' / script, *argv)

    action('new-work-directory', 'mkdir-new', path=str(work))
    action('download-cache', 'mkdir-new', path=str(cache))
    action('source-directory', 'mkdir-new', path=str(source))
    manifests = [
        ROOT / 'downloads.ini', platform_tree / 'downloads.x64.ini',
        platform_tree / 'downloads.ini', ROOT / 'deps.ini'
    ]
    utility('retrieve', 'downloads.py', 'retrieve', '-i', *manifests, '-c', cache)
    utility('unpack', 'downloads.py', 'unpack', '-i', *manifests, '-c', cache, source)
    command('source-version',
            Path(__file__).resolve(), 'preflight', '--platform-tree', platform_tree,
            '--source-tree', source)
    utility('prune', 'prune_binaries.py', '--keep-build-dependencies', source,
            ROOT / 'pruning.list')
    utility('patches', 'patches.py', 'apply', '--no-fuzz', '--patch-bin',
            source / 'third_party/git/usr/bin/patch.exe', source, ROOT / 'patches',
            platform_tree / 'patches')
    utility('domains', 'domain_substitution.py', 'apply', '-r', ROOT / 'domain_regex.list', '-f',
            ROOT / 'domain_substitution.list', source)
    utility('names', 'name_substitution.py', '--sub', '-t', source, '--backup-path',
            work / 'namesubs.tar')
    utility('translations', 'i18n_apply.py', '-t', source)
    utility('version', 'helium_version.py', '--tree', ROOT, '--platform-tree', platform_tree,
            '--chromium-tree', source)
    # Upstream generates Windows resources first, then common resources. Copy inputs first.
    for name, tree, resource_list in [('windows', platform_tree, 'platform_resources.txt'),
                                      ('common', ROOT, 'helium_resources.txt')]:
        resources = work / f'{name}-resources'
        action(f'copy-{name}-resources',
               'copy-tree-new',
               source=str(tree / 'resources'),
               destination=str(resources))
        utility(f'generate-{name}-resources', 'generate_resources.py',
                resources / 'generate_resources.txt', resources)
        utility(f'replace-{name}-resources', 'replace_resources.py', resources / resource_list,
                resources, source)
    # Siso backend setup belongs to the deferred compilation route, not GN generation.
    action('output-directory', 'mkdir-new', path=str(source / 'out/Default'))
    # Record the recipe and digest, not flag contents that could contain API credentials.
    action('gn-arguments',
           'concat-utf8',
           inputs=[str(ROOT / 'flags.gn'),
                   str(platform_tree / 'flags.windows.gn')],
           separator='\n',
           append='\nis_official_build=true\nenable_winsparkle=false\nuse_remoteexec=false\n',
           destination=str(source / 'out/Default/args.gn'),
           sha256=hashlib.sha256(gn_arguments(platform_tree).encode('utf-8')).hexdigest())
    action(
        'gn-generate',
        'command',
        argv=[str(source / 'buildtools/win/gn.exe'), 'gen', 'out/Default', '--fail-on-unused-args'],
        cwd=str(source),
        environment={'DEPOT_TOOLS_WIN_TOOLCHAIN': '0'},
        requires='Verified x64 Visual Studio developer environment with local Windows SDK')
    return steps


def prepare(platform_tree, work):
    """Return a plan without creating directories or running preparation commands."""
    for tree in (ROOT, platform_tree):
        if work == tree or work.is_relative_to(tree) or tree.is_relative_to(work):
            raise ValueError('Work directory must be separate from both input repositories')
    if work.exists():
        raise ValueError(
            'Work directory already exists; choose a new directory, do not reuse stamps')
    report = preflight(platform_tree)
    report['preparation'] = 'PLANNED'
    report['execution'] = 'UNIMPLEMENTED_OWNER_DEFERRED'
    report['execution_contract'] = {
        'authorization': 'Source retrieval and preparation require separate owner authorization',
        'host': 'Native Windows x64; Linux/WSL identity checks do not satisfy this gate',
        'inputs': 'Keep both repositories unchanged; compare identities before and after execution',
        'environment': (
            'Use a reviewed local developer environment. Do not inherit remote execution, '
            'compiler cache, updater, signing, or service credentials.'),
        'failure': (
            'Stop on every nonzero exit or interruption. Retain stage logs, checksum '
            'sidecars, backups, input manifest, and failed work tree. Never resume by stamps.'),
    }
    report['steps'] = preparation_plan(platform_tree, work)
    return report


def main():
    """CLI boundary; there is deliberately no execution option."""
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    check = commands.add_parser('preflight', help='Read-only identity and patch-file checks')
    check.add_argument('--platform-tree', type=Path)
    check.add_argument('--source-tree', type=Path)
    prep = commands.add_parser('prepare',
                               help='Print a plan only; preparation execution is deferred')
    prep.add_argument('--platform-tree', type=Path, required=True)
    prep.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    try:
        platform_tree = args.platform_tree.resolve() if args.platform_tree is not None else None
        if args.command == 'preflight':
            source_tree = args.source_tree.resolve() if args.source_tree is not None else None
            result = preflight(platform_tree, source_tree)
        else:
            result = prepare(platform_tree, args.work_dir.resolve())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'ERROR: {error}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
