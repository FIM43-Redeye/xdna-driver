#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror \
	"$repo_dir/test/aie2_rw_access_wire_test.c" \
	-o "$tmp_dir/aie2-rw-access-wire-test"
"$tmp_dir/aie2-rw-access-wire-test"
