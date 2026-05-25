#!/bin/bash
set -euo pipefail

# --- Flag parsing ---

SHOW_PRINTS=0
VERBOSE=0
args=()
while [ $# -gt 0 ]; do
  case "$1" in
    --show-prints) SHOW_PRINTS=1 ;;
    --verbose)     VERBOSE=1 ;;
    *)             args+=("$1") ;;
  esac
  shift
done
set -- ${args[@]+"${args[@]}"}

# --- Usage ---

if [ $# -eq 0 ] && [ "${1:-}" != "--only" ]; then
  # Only show usage when invoked truly bare; otherwise run everything below.
  :
fi

show_usage() {
  cat <<'USAGE'

Usage
  ./test.sh                            Run the whole test suite
  ./test.sh --only <path> [...]        Run specific test(s)

Options
  --show-prints      Show print() output from Swift test code
  --verbose          Native verbose flag — passed through to swift test

Examples
  ./test.sh                            # run all tests
  ./test.sh --show-prints              # include print() output
  ./test.sh --verbose                  # swift test --verbose

  # Single test (non-parameterized tests need trailing "()")
  ./test.sh --only 'MicroMaxOnAppleSiliconTests/CoordinateTests/coordinateToFileRank()'
  ./test.sh --only 'MicroMaxOnAppleSiliconTests/EngineLifecycle/EngineTests/startEngineReturnsInitBanners()'

Notes
  The --only path must match the full @Suite nesting chain:
    TestTarget/OuterSuite/InnerSuite/TestMethod()
  Non-parameterized tests need trailing "()", parameterized use the label with ":"

USAGE
}

ROOT="$(cd "$(dirname "$0")" && pwd)"
WALL_START=$(date +%s)

# --- Shared test runner ---

TEST_COUNT=""

run_tests() {
  local rc=0 output
  local verbose_args=()
  [ "$VERBOSE" = "1" ] && verbose_args+=(--verbose)

  output=$(swift test \
    ${verbose_args[@]+"${verbose_args[@]}"} \
    "$@" 2>&1) || rc=$?

  if [ "$SHOW_PRINTS" = "1" ]; then
    echo "$output"
  else
    # Keep Swift Testing's emoji status lines + suite/test summaries
    echo "$output" | grep -E "^(􀟈|􁁛|􀢄|􀄵|Test |✓|✗|\*\*)" || true
    if [ "$rc" -ne 0 ]; then
      echo "$output" | grep -E "error:|fatal|exited with unexpected signal" || true
    fi
  fi

  TEST_COUNT=$(echo "$output" | grep -oE "Test run with [0-9]+ tests?" | grep -oE "[0-9]+" | head -1 || true)
  return "$rc"
}

# --- --only mode ---

if [ "${1:-}" = "--only" ]; then
  filters=()
  while [ $# -gt 0 ]; do
    if [ "$1" = "--only" ]; then
      shift
      [ $# -eq 0 ] && { echo "ERROR: --only requires a filter path"; exit 1; }
      filters+=("$1")
      shift
    else
      echo "ERROR: Unexpected argument '$1' (expected --only)"
      exit 1
    fi
  done

  echo "━━━ Running: ${filters[*]} ━━━"

  parts=()
  for f in "${filters[@]}"; do parts+=("${f#*/}"); done
  filter_regex="$(IFS='|'; echo "${parts[*]}")"

  rc=0
  run_tests --filter "$filter_regex" || rc=$?

  if [ "${TEST_COUNT:-0}" = "0" ] || [ -z "${TEST_COUNT}" ]; then
    echo ""
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "!! 0 TESTS RAN — THE FILTER PATH MAY BE WRONG       !!"
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo ""
    echo "Filters used: ${filters[*]}"
    echo ""
    echo "Walk the @Suite nesting chain in MicroMaxOnAppleSiliconTests.swift:"
    echo "  TestTarget/OuterSuite/InnerSuite/TestMethod()"
    echo ""
    exit 1
  fi

  if [ "$rc" -ne 0 ]; then
    echo "✗ $TEST_COUNT tests — FAILED"
  else
    echo "✓ $TEST_COUNT tests passed"
  fi
  exit "$rc"
fi

# --- Full run ---

echo "━━━ Running all tests ━━━"
rc=0
run_tests || rc=$?

WALL_END=$(date +%s)
WALL_ELAPSED=$(( WALL_END - WALL_START ))
WALL_MIN=$(( WALL_ELAPSED / 60 ))
WALL_SEC=$(( WALL_ELAPSED % 60 ))

echo ""
if [ "$rc" -eq 0 ]; then
  echo "━━━ All tests passed in ${WALL_MIN}m ${WALL_SEC}s ━━━"
else
  echo "━━━ FAILED (${WALL_MIN}m ${WALL_SEC}s) ━━━"
fi
exit "$rc"
