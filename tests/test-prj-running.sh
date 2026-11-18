#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# test-prj-running.sh - Wrapper entry for running DDE File Manager unit tests.
#
# Delegates to autotests/run-ut.sh, which builds the project, runs all
# ut-* test binaries, and generates a coverage report.
#
# After run-ut.sh finishes and build-autotests/ut-summary.json is
# generated, the summary is copied to build-ut/ut-summary.json
# (the build-ut directory is created if it does not exist).
#
# Coverage mode is the DEFAULT. An explicit mode flag overrides it:
#
# Usage:
#   ./tests/test-prj-running.sh              # build with coverage + run + report
#   ./tests/test-prj-running.sh --coverage   # same as above (explicit)
#   ./tests/test-prj-running.sh --run-only   # run tests without rebuilding, no coverage
#
# All arguments are forwarded to autotests/run-ut.sh unchanged.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
RUN_UT="${PROJECT_ROOT}/autotests/run-ut.sh"

if [ ! -x "${RUN_UT}" ]; then
    echo "ERROR: ${RUN_UT} not found or not executable" >&2
    exit 1
fi

# Default to coverage mode unless an explicit flag was given.
has_mode=false
for arg in "$@"; do
    case "$arg" in
        --coverage|--run-only|--help|-h) has_mode=true ;;
    esac
done

if [ "${has_mode}" = false ]; then
    echo "==> No mode flag given, enabling coverage mode by default."
    set -- "$@" --coverage
fi

# Run the underlying script, remembering its exit code so the summary
# copy below still runs even when tests fail.
set +e
bash "${RUN_UT}" "$@"
rc=$?
set -e

# Copy the generated summary to build-ut/ for downstream consumers
# (e.g. CI expecting build-ut/ut-summary.json).
SUMMARY_SRC="${PROJECT_ROOT}/build-autotests/ut-summary.json"
BUILD_UT_DIR="${PROJECT_ROOT}/build-ut"

if [ -f "${SUMMARY_SRC}" ]; then
    mkdir -p "${BUILD_UT_DIR}"
    cp -f "${SUMMARY_SRC}" "${BUILD_UT_DIR}/ut-summary.json"
    echo "==> Copied ut-summary.json to ${BUILD_UT_DIR}/ut-summary.json"
else
    echo "Warning: ${SUMMARY_SRC} not found, skip copying to build-ut" >&2
fi

exit "${rc}"
