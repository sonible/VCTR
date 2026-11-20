#!/usr/bin/env bash
# Runs VCTR's verification steps with bounded output, for humans and agents.
#
# Full logs go to <build dir>/verify/<step>.log. On the console you get one line per step, and on failure only the
# error lines and the tail of the log. Steps run in the order given and the script stops at the first failure.
#
# Usage: scripts/verify.sh [options]
#
#   -s STEPS    Comma-separated steps to run (default: $VCTR_VERIFY_STEPS or "build,test,headers").
#               Steps: configure, build, test, docs, headers, all
#   -b DIR      Build directory (default: $VCTR_BUILD_DIR or "build")
#   -f SPEC     Catch2 test spec, e.g. "[abs]" or "Vector*" (default: $VCTR_TEST_FILTER, empty = all tests)
#   -t N        Number of log lines to print on failure (default: $VCTR_TAIL_LINES or 40)
#   -h          Show this help
#
# Environment:
#   CMAKE_ARGS  Extra arguments passed to the configure step (word-split), e.g. "-G Ninja -D VCTR_AUTOLINK_IPP=1"
#   DOXYGEN     Doxygen binary (default: "doxygen")
#
# The configure step runs automatically when the build directory has no CMakeCache.txt yet.
# Exit status: 0 when all selected steps pass, 1 when a step fails, 2 on usage errors.

set -u

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root" || exit 2

steps="${VCTR_VERIFY_STEPS:-build,test,headers}"
build_dir="${VCTR_BUILD_DIR:-build}"
test_filter="${VCTR_TEST_FILTER:-}"
tail_lines="${VCTR_TAIL_LINES:-40}"
cmake_args="${CMAKE_ARGS:-}"
doxygen_bin="${DOXYGEN:-doxygen}"

usage ()
{
    sed -n '2,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

while getopts ":s:b:f:t:h" opt; do
    case "$opt" in
        s) steps="$OPTARG" ;;
        b) build_dir="$OPTARG" ;;
        f) test_filter="$OPTARG" ;;
        t) tail_lines="$OPTARG" ;;
        h) usage; exit 0 ;;
        *) echo "Unknown option: -$OPTARG" >&2; usage >&2; exit 2 ;;
    esac
done

if [ "$steps" = "all" ]; then
    steps="configure,build,test,docs,headers"
fi

log_dir="$build_dir/verify"
mkdir -p "$log_dir" || exit 2

# Prints the lines most likely to explain a failure, then the tail of the log.
show_failure ()
{
    local log="$1"
    echo "  --- error lines (max 20) ---"
    grep -E "error|Error|ERROR|FAILED|warning:" "$log" | head -n 20 | sed 's/^/  /'
    echo "  --- last $tail_lines lines ---"
    tail -n "$tail_lines" "$log" | sed 's/^/  /'
    echo "  full log: $log"
}

# run_step NAME COMMAND...: runs the command with output captured to the step's log.
run_step ()
{
    local name="$1"
    shift
    local log="$log_dir/$name.log"
    local start=$SECONDS
    local status

    "$@" > "$log" 2>&1
    status=$?

    if [ $status -eq 0 ]; then
        echo "PASS $name ($((SECONDS - start))s)"
        return 0
    fi

    echo "FAIL $name (exit $status, $((SECONDS - start))s)"
    show_failure "$log"
    return 1
}

step_configure ()
{
    # shellcheck disable=SC2086 # cmake_args is intentionally word-split
    run_step configure cmake . -B "$build_dir" -D VCTR_BUILD_TEST=1 \
        -D CMAKE_PROJECT_TOP_LEVEL_INCLUDES=ext/cmake-conan/conan_provider.cmake $cmake_args
}

step_build ()
{
    run_step build cmake --build "$build_dir" --target vctr_test --parallel
}

step_test ()
{
    local binary="$build_dir/test/TestMain/vctr_test"
    local args=(-r compact --colour-mode none)
    if [ ! -x "$binary" ]; then
        echo "FAIL test: $binary not found (run the build step first)"
        return 1
    fi
    if [ -n "$test_filter" ]; then
        args+=("$test_filter")
    fi
    run_step test "$binary" "${args[@]}"
}

step_docs ()
{
    local log="$log_dir/docs.log"
    local start=$SECONDS
    if ! command -v "$doxygen_bin" > /dev/null 2>&1; then
        echo "FAIL docs: '$doxygen_bin' not found (set DOXYGEN=/path/to/doxygen)"
        return 1
    fi
    "$doxygen_bin" Doxyfile > "$log" 2>&1
    local status=$?
    local warnings
    warnings=$(grep -c "warning:" "$log")
    if [ $status -eq 0 ]; then
        echo "PASS docs ($((SECONDS - start))s, $warnings warnings)"
        return 0
    fi
    echo "FAIL docs (exit $status, $((SECONDS - start))s, $warnings warnings)"
    show_failure "$log"
    return 1
}

step_headers ()
{
    run_step headers python3 .claude/skills/fix-file-headers/fix_headers.py --check
}

# Build directory is needed by build/test; configure it automatically on first use.
needs_configure=0
case ",$steps," in
    *,configure,*) ;;
    *,build,*|*,test,*) [ -f "$build_dir/CMakeCache.txt" ] || needs_configure=1 ;;
esac

if [ $needs_configure -eq 1 ]; then
    echo "NOTE  no $build_dir/CMakeCache.txt, running configure first"
    step_configure || exit 1
fi

IFS=',' read -r -a step_list <<< "$steps"
for step in "${step_list[@]}"; do
    case "$step" in
        configure|build|test|docs|headers) ;;
        *) echo "Unknown step: '$step' (valid: configure, build, test, docs, headers, all)" >&2; exit 2 ;;
    esac
done

for step in "${step_list[@]}"; do
    case "$step" in
        configure) step_configure || exit 1 ;;
        build)     step_build || exit 1 ;;
        test)      step_test || exit 1 ;;
        docs)      step_docs || exit 1 ;;
        headers)   step_headers || exit 1 ;;
    esac
done

echo "All selected steps passed."
exit 0
