# SPDX-License-Identifier: GPL-3.0-only
# Copyright 2026 The Helium Arcified Authors
"""VERSION boundary checks only. These are not native browser tests."""

import unittest

import audit

VERSION = ('MAJOR=155\nMINOR=0\nBUILD=8059\nPATCH=79\n'
           'HELIUM_MAJOR=0\nHELIUM_MINOR=19\nHELIUM_PATCH=2\nHELIUM_PLATFORM=1\n')


class VersionTests(unittest.TestCase):
    """Malformed source metadata cannot silently select another engine identity."""

    def test_reads_both_engine_identities(self):
        """Keep Chromium and downstream version namespaces distinct."""
        self.assertEqual(audit.prepared_version(VERSION), ('155.0.8059.79', '0.19.2.1'))

    def test_rejects_duplicate_marker(self):
        """Do not silently accept last-writer-wins source metadata."""
        with self.assertRaises(ValueError):
            audit.prepared_version(VERSION + 'PATCH=80\n')

    def test_rejects_partial_downstream_version(self):
        """An unstamped or partially prepared source is not a prepared identity."""
        with self.assertRaises(KeyError):
            audit.prepared_version(VERSION.replace('HELIUM_PLATFORM=1\n', ''))

    def test_rejects_non_numeric_marker(self):
        """Reject invalid source markers at the file boundary."""
        with self.assertRaises(ValueError):
            audit.prepared_version(VERSION.replace('PATCH=79\n', 'PATCH=release\n'))


if __name__ == '__main__':
    unittest.main()
