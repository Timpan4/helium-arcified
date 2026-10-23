# -*- coding: UTF-8 -*-

# Copyright (c) 2020 The ungoogled-chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.ungoogled_chromium file.
"""Test check_patch_files.py"""

import logging
import tempfile
import sys
from pathlib import Path, PureWindowsPath
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent / 'utils'))
from _common import ENCODING, get_logger, set_logging_level

sys.path.pop(0)

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from check_patch_files import check_series_duplicates, check_unused_patches

sys.path.pop(0)


def test_check_series_duplicates():
    """Test check_series_duplicates"""

    set_logging_level(logging.DEBUG)

    with tempfile.TemporaryDirectory() as tmpdirname:
        patches_dir = Path(tmpdirname)
        series_path = Path(tmpdirname, 'series')

        get_logger().info('Check no duplicates')
        series_path.write_text('\n'.join([
            'a.patch',
            'b.patch',
            'c.patch',
        ]), encoding=ENCODING)
        assert not check_series_duplicates(patches_dir)

        get_logger().info('Check duplicates')
        series_path.write_text('\n'.join([
            'a.patch',
            'b.patch',
            'c.patch',
            'a.patch',
        ]),
                               encoding=ENCODING)
        assert check_series_duplicates(patches_dir)


def test_check_unused_patches_windows_paths():
    """Compare native Windows filesystem paths with slash-separated series entries."""
    # Retain this path-semantics fixture for review alongside native tooling fixtures.
    patches_dir = Path(tempfile.mkdtemp(prefix='arcified-windows-patch-paths-'))
    (patches_dir / 'metadata').mkdir()
    series = PureWindowsPath('metadata/series')
    (patches_dir / series).write_text('group/change.patch\n', encoding=ENCODING)

    def windows_entry(name):
        entry = mock.Mock()
        entry.is_dir.return_value = False
        entry.suffix = PureWindowsPath(name).suffix
        entry.relative_to.return_value = PureWindowsPath(name)
        return entry

    entries = [windows_entry('metadata/series'), windows_entry('group/change.patch')]
    with mock.patch.object(Path, 'rglob', return_value=entries):
        assert not check_unused_patches(patches_dir, series)
    entries.extend([windows_entry('z/unused.patch'), windows_entry('a/unused.patch')])
    with mock.patch.object(Path, 'rglob', return_value=entries), \
         mock.patch('check_patch_files.get_logger') as logger:
        assert check_unused_patches(patches_dir, series)
    assert logger.return_value.warning.call_args_list == [
        mock.call('Unused patch: %s', 'a/unused.patch'),
        mock.call('Unused patch: %s', 'z/unused.patch'),
    ]


if __name__ == '__main__':
    test_check_series_duplicates()
    test_check_unused_patches_windows_paths()
