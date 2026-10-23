#!/usr/bin/env bash
set -euo pipefail

feature_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repo_root=$(git -C "$feature_dir" rev-parse --show-toplevel)
test_output=$(mktemp -d)
trap 'rm -rf -- "$test_output"' EXIT

"${CXX:-clang++}" -std=c++17 -Wall -Wextra -Werror -pedantic "$@" \
  "$repo_root/arcified/native/model/lifecycle.cc" \
  "$feature_dir/../portability/format.cc" \
  "$feature_dir/converter.cc" "$feature_dir/converter_test.cc" \
  -o "$test_output/converter-test"
"$test_output/converter-test" "$feature_dir/fixtures/snapshots.json"
