#!/usr/bin/env python3
"""Report macOS input identity and a source preparation plan. Never execute the plan."""

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
# Newest inspected helium-macos main revision with this fork's exact engine version.
MACOS_REVISION = '9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28'
HELIUM_REVISION = 'b5145ab1047297970aceb9ca9f48722f6926cc66'
CHROMIUM_VERSION = '155.0.8059.79'
ARCHITECTURES = ('arm64', 'x64')


def preflight(platform_tree=None, source_tree=None):
    """Check local working inputs without probing or claiming a macOS toolchain."""
    version = (ROOT / 'chromium_version.txt').read_text(encoding='utf-8').strip()
    if version != CHROMIUM_VERSION:
        raise ValueError('Review and update the macOS tooling pin for the new Chromium version')
    report = {
        'chromium_version': version,
        'host': {
            'system': platform.system(),
            'machine': platform.machine(),
            'python': platform.python_version(),
        },
        'fork': input_identity.fork_identity(ROOT),
        'toolchain': {
            'status': 'UNVERIFIED',
            'native_macos_host': platform.system() == 'Darwin'
            and platform.machine() in ('arm64', 'x86_64'),
            'supported_host_architectures': list(ARCHITECTURES),
            'supported_target_architectures': list(ARCHITECTURES),
            'scope': ('No Xcode, SDK, Metal, APFS, extractor, compiler, GN, signing, '
                      'or macOS executable was probed. Host identity alone proves none of these.'),
        },
        'native_acceptance': 'OPEN',
        'source_preparation': 'OPEN',
        'patch_application': 'OPEN',
        'gn_generation': 'OPEN',
        'native_build': 'OPEN',
        'native_launch': 'OPEN',
    }
    if platform_tree is not None:
        if input_identity.git(platform_tree, 'rev-parse', 'HEAD') != MACOS_REVISION:
            raise ValueError(f'macOS tooling must be checked out at {MACOS_REVISION}')
        if input_identity.git(platform_tree, 'rev-parse',
                              'HEAD:helium-chromium') != HELIUM_REVISION:
            raise ValueError('Unexpected macOS tooling submodule identity')
        report['macos'] = input_identity.tree_identity(platform_tree, [
            'build.sh',
            'env.sh',
            'docs/building.md',
            'downloads.ini',
            'downloads.arm64.ini',
            'downloads.x64.ini',
            'flags.macos.gn',
            'revision.txt',
            'patches',
            'resources',
            'devutils',
        ])
        if report['macos']['tracked_changes'] or input_identity.git(
                platform_tree, 'ls-files', '--others', '--exclude-standard'):
            raise ValueError('macOS tooling has changes; use a clean pinned checkout')
        report['macos']['upstream_helium_commit'] = HELIUM_REVISION
        report['macos']['patch_source'] = 'Explicit downstream fork, not helium-chromium submodule'
    if source_tree is not None:
        report['source'] = input_identity.source_identity(source_tree, version)
    return report


def gn_arguments(platform_tree, target_arch):
    """Describe release flags without ambient updater, signing, or cache configuration."""
    return ((ROOT / 'flags.gn').read_text(encoding='utf-8') + '\n' +
            (platform_tree / 'flags.macos.gn').read_text(encoding='utf-8') +
            f'\ntarget_cpu = "{target_arch}"\nchrome_pgo_phase=2\n'
            'enable_sparkle=false\ninclude_branded_entitlements=false\n'
            'use_remoteexec=false\ncc_wrapper=""\n')


def preparation_plan(platform_tree, work, host_arch, target_arch):
    """Describe the pinned host-archive route, using this fork for all common inputs."""
    source = work / 'src'
    cache = work / 'download_cache'
    steps = []

    def action(name, operation, **fields):
        steps.append({'name': name, 'operation': operation, 'status': 'NOT_RUN', **fields})

    def command(name, script, *argv, cwd=ROOT):
        action(name, 'command', argv=list(map(str, [sys.executable, script, *argv])), cwd=str(cwd))

    def utility(name, script, *argv):
        command(name, ROOT / 'utils' / script, *argv)

    action('new-work-directory', 'mkdir-new', path=str(work))
    action('download-cache', 'mkdir-new', path=str(cache))
    action('source-directory', 'mkdir-new', path=str(source))
    # The archive contains HOST executables, independently of target_cpu.
    manifests = [
        ROOT / 'downloads.ini', platform_tree / f'downloads.{host_arch}.ini',
        platform_tree / 'downloads.ini', ROOT / 'deps.ini'
    ]
    utility('retrieve', 'downloads.py', 'retrieve', '-i', *manifests, '-c', cache)
    utility('unpack', 'downloads.py', 'unpack', '-i', *manifests, '-c', cache, source)
    command('source-version',
            Path(__file__).resolve(), 'preflight', '--platform-tree', platform_tree,
            '--source-tree', source)
    utility('prune', 'prune_binaries.py', '--keep-contingent-paths', source, ROOT / 'pruning.list')
    utility('patches', 'patches.py', 'apply', '--no-fuzz', source, ROOT / 'patches',
            platform_tree / 'patches')
    utility('domains', 'domain_substitution.py', 'apply', '-r', ROOT / 'domain_regex.list', '-f',
            ROOT / 'domain_substitution.list', '-c', work / 'subs.tar.gz', source)
    utility('names', 'name_substitution.py', '--sub', '-t', source, '--backup-path',
            work / 'namesubs.tar')
    utility('translations', 'i18n_apply.py', '-t', source)
    utility('version', 'helium_version.py', '--tree', ROOT, '--platform-tree', platform_tree,
            '--chromium-tree', source)
    # Upstream generates common resources, then installs platform resources before common ones.
    for name, tree in [('macos', platform_tree), ('common', ROOT)]:
        action(f'copy-{name}-resources',
               'copy-tree-new',
               source=str(tree / 'resources'),
               destination=str(work / f'{name}-resources'))
    utility('generate-common-resources', 'generate_resources.py',
            work / 'common-resources/generate_resources.txt', work / 'common-resources')
    for name, resource_list in [('macos', 'platform_resources.txt'),
                                ('common', 'helium_resources.txt')]:
        resources = work / f'{name}-resources'
        utility(f'replace-{name}-resources', 'replace_resources.py', resources / resource_list,
                resources, source)
    action('output-directory', 'mkdir-new', path=str(source / 'out/Default'))
    # Publish the recipe and digest, not flag contents that could contain API credentials.
    action('gn-arguments',
           'concat-utf8',
           inputs=[str(ROOT / 'flags.gn'),
                   str(platform_tree / 'flags.macos.gn')],
           separator='\n',
           append=(f'\ntarget_cpu = "{target_arch}"\nchrome_pgo_phase=2\n'
                   'enable_sparkle=false\ninclude_branded_entitlements=false\n'
                   'use_remoteexec=false\ncc_wrapper=""\n'),
           destination=str(source / 'out/Default/args.gn'),
           sha256=hashlib.sha256(gn_arguments(platform_tree,
                                              target_arch).encode('utf-8')).hexdigest())
    # Backend configuration belongs to the deferred compilation route, not GN generation.
    action('gn-generate',
           'command',
           argv=[str(source / 'buildtools/mac/gn'), 'gen', 'out/Default', '--fail-on-unused-args'],
           cwd=str(source),
           requires='Verified native macOS environment, host-matching tools, Xcode and macOS SDK')
    return steps


def _has_input_ancestor(work, tree):
    """Check existing ancestors by identity, even when new work has missing parents."""
    for ancestor in (work, *work.parents):
        try:
            if ancestor.samefile(tree):
                return True
        except FileNotFoundError:
            continue
    return False


def prepare(platform_tree, work, host_arch, target_arch):
    """Validate planning inputs and return a plan without creating or executing anything."""
    if host_arch not in ARCHITECTURES or target_arch not in ARCHITECTURES:
        raise ValueError('Host and target architectures must be arm64 or x64')
    for tree in (ROOT, platform_tree):
        if (work == tree or work.is_relative_to(tree) or tree.is_relative_to(work)
                or _has_input_ancestor(work, tree)):
            raise ValueError('Work directory must be separate from both input repositories')
    # Chromium's pinned mac_build_instructions.md requires a source path without spaces.
    if ' ' in str(work):
        raise ValueError('Chromium macOS source paths must not contain spaces')
    if work.exists():
        raise ValueError(
            'Work directory already exists; choose a new directory, do not reuse stamps')
    report = preflight(platform_tree)
    report['target'] = f'mac-{target_arch}'
    report['archive_host_architecture'] = host_arch
    report['preparation'] = 'PLANNED'
    report['execution'] = 'UNIMPLEMENTED_OWNER_DEFERRED'
    report['execution_contract'] = {
        'authorization': 'Source retrieval and preparation require separate owner authorization',
        'host': ('Native macOS with the declared host architecture. '
                 'Linux identity checks do not satisfy this gate.'),
        'inputs': 'Keep both repositories unchanged; compare identities before and after execution',
        'environment': ('Use a reviewed local environment and verified APFS work volume. '
                        'Do not inherit remote execution, compiler cache, updater, signing, '
                        'notarization, provisioning, or service credentials.'),
        'failure': (
            'Stop on every nonzero exit or interruption. Retain stage logs, checksum '
            'sidecars, backups, input manifest, and failed work tree. Never resume by stamps.'),
    }
    report['steps'] = preparation_plan(platform_tree, work, host_arch, target_arch)
    return report


def main():
    """CLI boundary; all commands are read-only and there is no execution option."""
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    check = commands.add_parser('preflight', help='Read-only identity and patch-file checks')
    check.add_argument('--platform-tree', type=Path)
    check.add_argument('--source-tree', type=Path)
    prep = commands.add_parser('prepare', help='Print a plan only; execution is deferred')
    prep.add_argument('--platform-tree', type=Path, required=True)
    prep.add_argument('--work-dir', type=Path, required=True)
    prep.add_argument('--host-arch',
                      choices=ARCHITECTURES,
                      required=True,
                      help='Intended macOS host, x64 means Intel x86_64; selects the tools archive')
    prep.add_argument('--target-arch',
                      choices=ARCHITECTURES,
                      required=True,
                      help='Chromium target_cpu, independent of the host tools archive')
    args = parser.parse_args()
    try:
        platform_tree = args.platform_tree.resolve() if args.platform_tree is not None else None
        if args.command == 'preflight':
            source_tree = args.source_tree.resolve() if args.source_tree is not None else None
            result = preflight(platform_tree, source_tree)
        else:
            result = prepare(platform_tree, args.work_dir.resolve(), args.host_arch,
                             args.target_arch)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'ERROR: {error}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
