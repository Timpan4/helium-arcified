#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../../../.." && pwd)
cd -- "$repo_root"
test_source=arcified/native/integration/layout/layout_split_composition_test.cc
sources=(
  arcified/native/model/lifecycle.cc
  arcified/native/features/layout/focus_mode/focus_mode.cc
  arcified/native/features/layout/sidebar/sidebar.cc
  arcified/native/features/layout/tab_presentation/tab_presentation.cc
  arcified/native/features/splits/layout.cc
  arcified/native/features/splits/geometry.cc
  arcified/native/features/motion/presentation_transition.cc
  "$test_source"
)
flags=(-std=c++17 -Wall -Wextra -Werror -pedantic)
output_dir=$(mktemp -d "${TMPDIR:-/tmp}/arcified-layout-conformance.XXXXXXXX")
trap 'rm -rf -- "$output_dir"' EXIT

clang++ "${flags[@]}" "${sources[@]}" -o "$output_dir/strict"
"$output_dir/strict"
clang++ "${flags[@]}" -g -fsanitize=address,undefined \
  -fno-sanitize-recover=all -fno-omit-frame-pointer \
  "${sources[@]}" -o "$output_dir/sanitized"
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  "$output_dir/sanitized"
clang-format --style=Chromium --dry-run --Werror "$test_source"
clang-tidy --checks='-*,clang-analyzer-*,bugprone-*' \
  --warnings-as-errors='*' "$test_source" -- "${flags[@]}"
