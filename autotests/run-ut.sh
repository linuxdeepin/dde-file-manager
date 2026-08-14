#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# run-ut.sh - Build, run, and optionally generate coverage for DDE File Manager unit tests.
#
# Usage:
#   ./autotests/run-ut.sh              # build + run tests
#   ./autotests/run-ut.sh --coverage   # build with coverage instrumentation + run + generate report
#   ./autotests/run-ut.sh --run-only   # run tests without rebuilding
#
set -euo pipefail

export SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
export BUILD_DIR="${PROJECT_ROOT}/build-autotests"
export builddir=build-autotests
export reportdir=build-autotests
export projectdir="${PROJECT_ROOT}"
COVERAGE=false
RUN_ONLY=false

# Parse arguments
for arg in "$@"; do
    case "$arg" in
        --coverage) COVERAGE=true ;;
        --run-only) RUN_ONLY=true ;;
        --help|-h)
            echo "Usage: $0 [--coverage] [--run-only]"
            echo "  --coverage   Enable gcov coverage instrumentation and generate HTML report"
            echo "  --run-only   Run existing tests without rebuilding"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            exit 1
            ;;
    esac
done

# Step 1: Configure
if [ "${RUN_ONLY}" = false ]; then
    echo "==> [1/4] Configuring..."
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"

    CMAKE_ARGS=(
        -S "${PROJECT_ROOT}"
        -B "${BUILD_DIR}"
        -DCMAKE_BUILD_TYPE=Debug
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DOPT_ENABLE_BUILD_UT=ON
    )

    if [ "${COVERAGE}" = true ]; then
        CMAKE_ARGS+=(
            -DDFM_ENABLE_COVERAGE=ON
            -DCMAKE_CXX_FLAGS="--coverage -O0 -fno-inline"
            -DCMAKE_EXE_LINKER_FLAGS="--coverage"
        )
    fi

    cmake "${CMAKE_ARGS[@]}"

    # Step 2: Build
    echo "==> [2/4] Building..."
    cmake --build "${BUILD_DIR}" -j"$(nproc)"
fi

# Step 3: Run tests and collect results
echo "==> [3/4] Running tests..."
cd "${BUILD_DIR}"

# Use offscreen Qt platform to avoid display dependencies.
export QT_QPA_PLATFORM="offscreen"

# Ensure locally-built libraries (dfm6-base, extractor) are found at runtime
# ahead of older system-installed copies, preventing symbol-resolution
# failures (e.g. ut-textindex needing newer dfm6-base symbols).
export LD_LIBRARY_PATH="${BUILD_DIR}/src/dfm-base:${BUILD_DIR}/src/apps/dde-file-manager-extractor${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

# Discover test binaries
GTEST_RESULTS_DIR="${BUILD_DIR}/gtest-results"
mkdir -p "${GTEST_RESULTS_DIR}"

test_bins=()
while IFS= read -r -d '' bin; do
    test_bins+=("$bin")
done < <(find "${BUILD_DIR}/autotests" -type f -executable -name 'ut-*' -print0 | sort -z)

if [ ${#test_bins[@]} -eq 0 ]; then
    echo "ERROR: No test binaries found under ${BUILD_DIR}/autotests"
    exit 1
fi

echo "Found ${#test_bins[@]} test binaries:"
for bin in "${test_bins[@]}"; do
    echo "  $(basename "$bin")"
done
echo ""

# Run each test binary with gtest XML output
# Temporarily disable 'set -e' so a failing test binary doesn't abort the script;
# we track failures via any_failed and report them at the end.
any_failed=0
set +e
for bin in "${test_bins[@]}"; do
    name=$(basename "$bin")
    echo "---- Running ${name} ----"
    if "$bin" --gtest_output="xml:${GTEST_RESULTS_DIR}/${name}.xml"; then
        echo "  ${name}: PASSED"
    else
        echo "  ${name}: FAILED"
        any_failed=1
    fi
done
set -e

echo ""
if [ "$any_failed" -eq 0 ]; then
    echo "All test binaries passed."
else
    echo "Some test binaries FAILED."
fi

# Step 4: Coverage report
if [ "${COVERAGE}" = true ]; then
    echo "==> [4/4] Generating coverage report..."

    COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${COVERAGE_DIR}"

    # --- Baseline from .gcno (compiled-but-unrun source files) ---
    # lcov --capture only reads .gcda runtime data and silently drops every
    # file that was compiled with --coverage but never executed. That shrinks
    # the denominator and inflates the coverage %. Capturing an initial
    # baseline from the .gcno files and merging it back in forces those files
    # into the report at 0%, so the figure reflects the real whole-project
    # denominator rather than only what tests happened to touch.
    lcov --capture --initial --directory "${BUILD_DIR}" \
        --output-file "${COVERAGE_DIR}/baseline.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

    # --- Runtime coverage from .gcda (after test execution) ---
    lcov --capture --directory "${BUILD_DIR}" \
        --output-file "${COVERAGE_DIR}/run.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

    # --- Merge: keep real hit counts; files only in the baseline stay at 0% ---
    if [ -s "${COVERAGE_DIR}/baseline.info" ]; then
        lcov --add-tracefile "${COVERAGE_DIR}/baseline.info" \
            --add-tracefile "${COVERAGE_DIR}/run.info" \
            --output-file "${COVERAGE_DIR}/coverage.info" \
            --rc lcov_branch_coverage=1 2>/dev/null || true
    else
        cp "${COVERAGE_DIR}/run.info" "${COVERAGE_DIR}/coverage.info"
    fi

    # Filter to only include project source files
    lcov --remove "${COVERAGE_DIR}/coverage.info" \
        '*/3rdparty/*' \
        '*/testutils/*' \
        '*/autotests/*' \
        '*/build*' \
        '/usr/*' \
        --output-file "${COVERAGE_DIR}/coverage-filtered.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

    # Generate HTML report
    genhtml "${COVERAGE_DIR}/coverage-filtered.info" \
        --output-directory "${COVERAGE_DIR}/html" \
        --branch-coverage \
        --legend \
        --title "DDE File Manager - Unit Test Coverage" 2>/dev/null || true

    # Print summary
    echo ""
    echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
    lcov --summary "${COVERAGE_DIR}/coverage-filtered.info" --rc lcov_branch_coverage=1 2>/dev/null || \
        echo "Warning: Could not generate coverage summary (lcov may not be installed)"
fi

# Step 5: Generate summary JSON
echo "==> Generating summary JSON: ${BUILD_DIR}/ut-summary.json"

# dde-file-manager 的 gtest XML 和 coverage info 路径与默认不同，通过环境变量覆盖
export GTEST_XML_DIR="${BUILD_DIR}/gtest-results"
if [ "${COVERAGE}" = true ]; then
    export COVERAGE_INFO="${BUILD_DIR}/coverage/coverage-filtered.info"
fi

python3 "${SCRIPT_DIR}/gen-ut-summary.py"

echo ""
echo "Done."
