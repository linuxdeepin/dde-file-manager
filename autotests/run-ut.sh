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

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-autotests"
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

# Step 3: Run tests
echo "==> [3/4] Running tests..."
cd "${BUILD_DIR}"
ctest --output-on-failure --timeout 60

# Step 4: Coverage report
if [ "${COVERAGE}" = true ]; then
    echo "==> [4/4] Generating coverage report..."

    COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${COVERAGE_DIR}"

    # Collect coverage data
    lcov --capture --directory "${BUILD_DIR}" --output-file "${COVERAGE_DIR}/coverage.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

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

echo ""
echo "Done."
