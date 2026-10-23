#!/usr/bin/env bash
set -euo pipefail

feature_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/arcified-workspace-import.XXXXXXXX")
trap 'rm -rf -- "$test_dir"' EXIT

flags=(-std=c++17 -Wall -Wextra -Werror -pedantic)
case "${1:-}" in
  "") ;;
  --sanitize)
    flags+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
    # Sanitizer diagnostics must fail this check, including recoverable UB.
    export ASAN_OPTIONS=halt_on_error=1
    export UBSAN_OPTIONS=halt_on_error=1
    ;;
  *) printf 'Usage: bash run_checks.sh [--sanitize]\n' >&2; exit 2 ;;
esac

clang-format --style=Chromium --dry-run --Werror \
  "$feature_dir/planner.h" "$feature_dir/planner.cc" \
  "$feature_dir/planner_test.cc"
clang++ --driver-mode=g++ "${flags[@]}" \
  "$feature_dir/../../model/lifecycle.cc" \
  "$feature_dir/../portability/format.cc" \
  "$feature_dir/../prototype_migration/converter.cc" \
  "$feature_dir/planner.cc" "$feature_dir/planner_test.cc" \
  -o "$test_dir/planner_test"
"$test_dir/planner_test"
