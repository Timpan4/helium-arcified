# SPDX-License-Identifier: GPL-3.0-only
# Copyright 2026 The Helium Arcified Authors
"""Record the actual fork inputs and prepared engine markers without preparing/building."""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / 'arcified/native/tooling'))
import input_identity  # pylint: disable=wrong-import-position


def prepared_version(text):
    """Reject ambiguous external VERSION markers, including partial Helium stamps."""
    fields = {}
    for line in text.splitlines():
        key, value = line.split('=', 1)
        if key in fields or re.fullmatch(r'[0-9]+', value) is None:
            raise ValueError('Duplicate or non-numeric prepared VERSION marker')
        fields[key] = value
    chromium = '.'.join(fields[key] for key in ('MAJOR', 'MINOR', 'BUILD', 'PATCH'))
    helium = '.'.join(fields[key] for key in
                      ('HELIUM_MAJOR', 'HELIUM_MINOR', 'HELIUM_PATCH', 'HELIUM_PLATFORM'))
    return chromium, helium


def audit(source):
    """Reuse the existing patch validator and ordered fork identity implementation."""
    chromium, helium = prepared_version((source / 'chrome/VERSION').read_text(encoding='utf-8'))
    expected = (ROOT / 'chromium_version.txt').read_text(encoding='utf-8').strip()
    if chromium != expected:
        raise ValueError(f'Prepared Chromium {chromium} does not match fork pin {expected}')
    return {
        'issue': 'https://github.com/Timpan4/helium-arcified/issues/41',
        'fork': input_identity.fork_identity(ROOT),
        'prepared_source': {
            'path': str(source),
            'chromium_version': chromium,
            'helium_version': helium,
            'files_sha256': {
                name: input_identity.digest(source / name)
                for name in ('chrome/VERSION', 'DEPS', 'chrome/browser/profiles/BUILD.gn',
                             'chrome/test/BUILD.gn')
            },
            'scope': 'Selected file identities only; not a pristine-tree or archive attestation.',
        },
        'native_build': 'OPEN',
        'native_acceptance': dict.fromkeys(('linux', 'windows', 'macos'), 'OPEN'),
        'update_execution': 'OPEN',
        'security_review': 'See security-review.json; refresh authoritative sources per update.',
    }


def main():
    """Emit provenance on stdout; diagnostics on stderr, with failure propagated."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-tree', type=Path, required=True)
    args = parser.parse_args()
    try:
        report = audit(args.source_tree.resolve())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'ERROR: {error}\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
