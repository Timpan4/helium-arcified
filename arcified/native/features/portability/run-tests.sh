#!/usr/bin/env bash
set -euo pipefail

feature_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
test_binary=$(mktemp "${TMPDIR:-/tmp}/arcified-portability-test.XXXXXX")
trap 'rm -f -- "$test_binary"' EXIT

"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  "$feature_dir/format.cc" "$feature_dir/format_test.cc" \
  "$feature_dir/../../model/lifecycle.cc" -o "$test_binary"
"$test_binary"
