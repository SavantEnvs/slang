#!/usr/bin/env bash
#
# mayhem/test.sh — RUN this repo's OWN functional test suite (already built by mayhem/build.sh).
# exit 0 = pass. EDIT per repo. PATCH-grade oracle: after an agent patches the source, the grader
# rebuilds (build.sh) then runs this. DELETE this file if the repo has no meaningful tests.
#
# IMPORTANT:
#  * Must assert BEHAVIOR/OUTPUT, not just exit status. The oracle has to check asserted values /
#    golden-output diffs / known-answer results — so a PATCH that "fixes" a bug by making the program
#    exit(0) (or any no-op) FAILS here. Running inputs and checking only "exit 0 / didn't crash" is
#    NOT a functional test (it's trivially reward-hackable) — use the project's real assertion suite.
#  * Do NOT build here — mayhem/build.sh already compiled the test suite (with the project's normal
#    flags). This script only RUNS the pre-built tests and reports counts. If the test runner is
#    missing, that's a build.sh bug — fail loudly rather than silently rebuilding.
#  * REQUIRED OUTPUT — a CTRF (https://ctrf.io) summary so Mayhem/the PATCH grader reads the counts:
#      - writes a CTRF JSON report to ${CTRF_REPORT:-$SRC/ctrf-report.json}, and
#      - prints a one-line `CTRF {...}` marker to stdout (same JSON, compact).
#    Only `results.summary` (with tests/passed/failed/pending/skipped/other) is required.
#    Use the emit_ctrf helper below; it computes tests = passed+failed+skipped and sets the exit
#    code (0 iff failed==0). Map your framework's output to passed/failed/skipped.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"   # build parallelism; env-overridable, falls back to nproc (use -j"$MAYHEM_JOBS")
cd "$SRC"

# emit_ctrf <tool> <passed> <failed> [skipped] [pending] [other]
# Writes a CTRF report (file + stdout `CTRF {...}` marker) and returns non-zero iff failed>0.
emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

T=build/mayhem-tests/bin
[ -x "$T/unittests" ] && [ -x "$T/slang" ] || { echo "test binaries missing: run mayhem/build.sh" >&2; exit 2; }
passed=0; failed=0

# 1) Upstream's Catch2 unit tests (thousands of known-answer parse/elaborate/eval assertions).
#    Compact reporter summary: "All tests passed (A assertions in N test cases)" or
#    "Failed F test cases, failed A assertions." (count of all cases from --list-tests).
log=$(mktemp)
"$T/unittests" --reporter compact --colour-mode none >"$log" 2>&1; rc=$?
tail -n 3 "$log"
total=$("$T/unittests" --list-tests --verbosity quiet --colour-mode none 2>/dev/null | grep -c . || true)
n=$(sed -n 's/^All tests passed (.* in \([0-9]*\) test case.*/\1/p' "$log" | tail -1)
if [ "$rc" -eq 0 ] && [ -n "$n" ] && [ "$n" -gt 0 ]; then
  passed=$((passed + n))
else
  f=$(sed -n 's/^Failed \([0-9]*\) test case.*/\1/p' "$log" | tail -1)
  [ -n "$f" ] && [ "$f" -gt 0 ] || f=1
  failed=$((failed + f)); p=$(( ${total:-0} - f )); [ "$p" -ge 0 ] || p=0
  passed=$((passed + p))
fi
rm -f "$log"

# 2) Golden checks of the `slang` CLI on a small design: a clean module must elaborate with
#    "Build succeeded", a type error must be reported with its diagnostic, and --ast-json must
#    contain the evaluated parameter value.
d=$(mktemp -d)
printf 'module top;\n  localparam int P = 3 * 7 + 1;\n  logic [P-1:0] x;\nendmodule\n' >"$d/ok.sv"
printf 'module top;\n  int a = undeclared_thing;\nendmodule\n' >"$d/bad.sv"
check() { if "$@"; then passed=$((passed + 1)); else echo "FAIL: $*" >&2; failed=$((failed + 1)); fi; }
"$T/slang" --color-diagnostics=false "$d/ok.sv" >"$d/ok.out" 2>&1
"$T/slang" --color-diagnostics=false "$d/bad.sv" >"$d/bad.out" 2>&1
check grep -q 'Build succeeded: 0 errors, 0 warnings' "$d/ok.out"
check grep -q "use of undeclared identifier 'undeclared_thing'" "$d/bad.out"
"$T/slang" -q "$d/ok.sv" --ast-json "$d/ast.json" >/dev/null 2>&1
check grep -q '"value": "22"' "$d/ast.json"
[ "$failed" -eq 0 ] || { cat "$d/ok.out" "$d/bad.out"; head -c 2000 "$d/ast.json"; } >&2
rm -rf "$d"

emit_ctrf "catch2+slang-cli" "$passed" "$failed" 0
