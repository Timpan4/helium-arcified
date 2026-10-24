#!/bin/sh
# Copyright 2026 The Arcified Authors
# SPDX-License-Identifier: GPL-3.0-only
set -eu

packet_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
source_root=${1:?Pass the read-only prepared Chromium source root}

cd "$packet_dir"
sha256sum -c SHA256SUMS
cmp prepared-base/chrome/test/BUILD.gn "$source_root/chrome/test/BUILD.gn"

# Confine all application checks to a fresh directory owned by this script.
release_check_dir=$(mktemp -d "$packet_dir/.packet-check.XXXXXX")
trap 'rm -rf -- "$release_check_dir"' EXIT HUP INT TERM
mkdir -p "$release_check_dir/chrome/test"
cp prepared-base/chrome/test/BUILD.gn "$release_check_dir/chrome/test/BUILD.gn"
GIT_CEILING_DIRECTORIES="$packet_dir" git -C "$release_check_dir" apply \
  --check --whitespace=error "$packet_dir/production.patch"
GIT_CEILING_DIRECTORIES="$packet_dir" git -C "$release_check_dir" apply \
  --whitespace=error "$packet_dir/production.patch"
cmp prepared-result/chrome/test/BUILD.gn "$release_check_dir/chrome/test/BUILD.gn"
cmp prepared-result/chrome/browser/arcified/release_linux_browsertest.cc \
  "$release_check_dir/chrome/browser/arcified/release_linux_browsertest.cc"
GIT_CEILING_DIRECTORIES="$packet_dir" git -C "$release_check_dir" apply \
  --reverse --check --whitespace=error "$packet_dir/production.patch"

clang-format --style=Chromium --dry-run --Werror \
  prepared-result/chrome/browser/arcified/release_linux_browsertest.cc
"$source_root/buildtools/linux64/gn" format --dry-run \
  prepared-result/chrome/test/BUILD.gn
printf '%s\n' 'PASS: file hashes, prepared base identity, sparse forward/reverse application, exact result comparison, C++ formatting and GN formatting.'
printf '%s\n' 'OPEN: native compilation/execution, installed-package acceptance and owner decisions.'
