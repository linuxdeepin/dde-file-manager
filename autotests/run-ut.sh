#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# run-ut.sh - Build, run, and optionally generate coverage for DDE File Manager
#             unit tests. Supports full runs and incremental (changed-files)
#             runs for CI pipelines.
#
# Usage:
#   ./autotests/run-ut.sh                        # full: fresh configure + build + run all
#   ./autotests/run-ut.sh --coverage             # full run with gcov coverage report
#   ./autotests/run-ut.sh --run-only             # run all tests without rebuilding
#
# Incremental mode (local minimal testing — only run suites affected by changes):
#   ./autotests/run-ut.sh --changed-files src/dfm-framework/lifecycle/plugin.cpp
#   ./autotests/run-ut.sh --changed-files src/dfm-framework/event/   # a directory
#   ./autotests/run-ut.sh --git-diff origin/master                   # diff vs branch
#   git diff --name-only origin/master | ./autotests/run-ut.sh --changed-files -
#   ./autotests/run-ut.sh --changed-files @changed.txt               # list from file
#   ./autotests/run-ut.sh --list --changed-files src/dfm-base        # dry run
#
# How mapping works:
#   CMake records "suite -> covered source dir" pairs into
#   build-autotests/ut-suite-map.tsv (via dfm_add_test(COVERS ...)).
#   A changed file selects every suite whose test dir or covered dir is a
#   path prefix of the file. Shared test infrastructure changes (test main,
#   stubs, CMake test macros) select all suites. Source files no suite maps
#   to follow --on-no-match (default: run everything — fail-safe).
#
# Exit codes: 0 ok/skipped | 1 build or test failure | 3 strict no-match | 4 bad usage
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-autotests"
SUITE_MAP="${BUILD_DIR}/ut-suite-map.tsv"
COVERAGE=false
RUN_ONLY=false
LIST_ONLY=false
INCREMENTAL=false
ON_NO_MATCH="all"          # all | skip | strict
MANUAL_SUITES=""
CHANGED_RAW=()

# Shared test infrastructure: a change here affects every suite
SHARED_TEST_PATTERNS=(
    "autotests/dfm_test_main.h"
    "autotests/dfm_asan_helper.h"
    "autotests/CMakeLists.txt"
    "autotests/libs/CMakeLists.txt"
    "autotests/libs/*/main.cpp"
    "autotests/libs/*/dfm_extension_stub.cpp"
    "autotests/libs/*/CMakeLists.txt"
    "cmake/DFMTestUtils.cmake"
    "3rdparty/testutils/*"
    "3rdparty/testutils/*/*"
)

# Files that never need unit tests
NONCODE_PATTERNS=(
    "debian/*" "debian/*/*" "debian/*/*/*" "debian/*/*/*/*"
    "docs/*" "docs/*/*"
    ".github/*" ".github/*/*"
    "*.md" "*.txt" "*.json" "*.xml" "*.ts" "*.qm" "*.qrc" "*.ui"
    "*.svg" "*.png" "*.jpg" "*.jpeg" "*.gif" "*.ico" "*.desktop" "*.service"
    "translations/*" "assets/*" "LICENSES/*"
)

usage() { grep '^#' "$0" | sed 's/^# \{0,1\}//' | head -60; exit 0; }

die() { echo "run-ut.sh: $*" >&2; exit 4; }

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
while [ $# -gt 0 ]; do
    case "$1" in
        --coverage)  COVERAGE=true ;;
        --run-only)  RUN_ONLY=true ;;
        --list|--dry-run) LIST_ONLY=true ;;
        --suites)    shift; MANUAL_SUITES="${1:-}"; [ -n "$MANUAL_SUITES" ] || die "--suites needs a list" ;;
        --suites=*)  MANUAL_SUITES="${1#*=}" ;;
        --on-no-match) shift
            case "${1:-}" in all|skip|strict) ON_NO_MATCH="$1" ;; *) die "invalid --on-no-match: ${1:-}" ;; esac ;;
        --on-no-match=*) 
            v="${1#*=}"; case "$v" in all|skip|strict) ON_NO_MATCH="$v" ;; *) die "invalid --on-no-match: $v" ;; esac ;;
        --build-dir) shift; BUILD_DIR="${1:-}"; SUITE_MAP="${BUILD_DIR}/ut-suite-map.tsv" ;;
        --git-diff)  shift
            ref="${1:-}"; [ -n "$ref" ] || die "--git-diff needs a ref"
            INCREMENTAL=true
            while IFS= read -r l; do [ -n "$l" ] && CHANGED_RAW+=("$l"); done < <(
                { git -C "${PROJECT_ROOT}" diff --name-only "${ref}...HEAD"
                  git -C "${PROJECT_ROOT}" diff --name-only --cached
                  git -C "${PROJECT_ROOT}" ls-files --others --exclude-standard
                } | sort -u ) ;;
        --changed-files)
            INCREMENTAL=true
            shift
            [ $# -gt 0 ] || die "--changed-files needs at least one path (or - / @file)"
            # Consume remaining args as the file list (other options must come first)
            while [ $# -gt 0 ]; do
                v="$1"; shift
                case "$v" in
                    -)  while IFS= read -r l; do [ -n "$l" ] && CHANGED_RAW+=("$l"); done ;;
                    @*) [ -f "${v:1}" ] || die "file not found: ${v:1}"
                        while IFS= read -r l; do [ -n "$l" ] && CHANGED_RAW+=("$l"); done < "${v:1}" ;;
                    *)  CHANGED_RAW+=("$v") ;;
                esac
            done ;;
        --help|-h)   usage ;;
        *)           die "unknown option: $1 (see --help)" ;;
    esac
    shift || true
done

# Normalize: strip project prefix / leading ./
norm_rel() {
    local p="$1"
    p="${p#"${PROJECT_ROOT}"/}"
    p="${p#./}"
    printf '%s' "$p"
}

glob_match() { # glob_match <path> <pattern...>  -> 0 if any pattern matches
    local p="$1"; shift
    local pat
    for pat in "$@"; do
        # shellcheck disable=SC2254
        case "$p" in
            $pat) return 0 ;;
        esac
    done
    return 1
}

is_noncode() { # is_noncode <relpath>  -> 0 if the file never needs tests
    local p="$1" base
    base="$(basename "$p")"
    # build files are code
    case "$base" in
        CMakeLists.txt|*.cmake|*.sh) return 1 ;;
    esac
    glob_match "$p" "${NONCODE_PATTERNS[@]}"
}

# ---------------------------------------------------------------------------
# Suite selection (incremental mode)
# ---------------------------------------------------------------------------
declare -a SELECTED=()
declare -A SELECTION_REASON

select_suite() { # select_suite <suite> <reason>
    if [ -z "${SELECTION_REASON[$1]+x}" ]; then
        SELECTED+=("$1")
        SELECTION_REASON[$1]="$2"
    elif [[ "${SELECTION_REASON[$1]}" != *"$2"* ]]; then
        SELECTION_REASON[$1]="${SELECTION_REASON[$1]}; $2"
    fi
}

select_all_suites() { # select_all_suites <reason>
    local suite
    while IFS=$'\t' read -r suite _; do
        [[ "$suite" == \#* || -z "$suite" ]] && continue
        select_suite "$suite" "$1"
    done < "${SUITE_MAP}"
}

ensure_suite_map() {
    if [ ! -f "${SUITE_MAP}" ]; then
        echo "==> Suite map not found, configuring to generate it..."
        configure_project
    fi
    [ -f "${SUITE_MAP}" ] || die "suite map still missing after configure: ${SUITE_MAP}"
}

# Map changed files to suites using ut-suite-map.tsv (longest-prefix match)
map_changed_to_suites() {
    local -a files=()
    local f p matched_any_code=false

    for f in "${CHANGED_RAW[@]}"; do
        # split on whitespace/comma
        IFS=$' \t,\n' read -ra toks <<< "$f"
        for t in "${toks[@]}"; do
            [ -n "$t" ] && files+=("$(norm_rel "$t")")
        done
    done
    [ ${#files[@]} -gt 0 ] || die "no changed files given"
    # dedupe (merge-commit diffs can list a path twice)
    mapfile -t files < <(printf '%s\n' "${files[@]}" | sort -u)

    echo "==> Mapping ${#files[@]} changed path(s) to test suites..."

    for p in "${files[@]}"; do
        # 1) Non-code files never trigger tests
        if is_noncode "$p"; then
            echo "    skip (non-code):    $p"
            continue
        fi
        matched_any_code=true

        # 2) Shared test infrastructure -> all suites
        if glob_match "$p" "${SHARED_TEST_PATTERNS[@]}"; then
            select_all_suites "shared test infra: $p"
            echo "    ALL suites (shared): $p"
            continue
        fi

        # 3) Prefix match against suite map (test dirs + COVERS dirs).
        #    Longest matching dir wins per suite; every matched suite is kept.
        local suite dir best_len best_suite= match_len
        local hit=false
        while IFS=$'\t' read -r suite dir; do
            [[ "$suite" == \#* || -z "$suite" ]] && continue
            dir="${dir%/}"
            if [ "$p" = "$dir" ] || [[ "$p" == "$dir"/* ]] || [[ "$dir" == "$p"/* ]]; then
                select_suite "$suite" "$p"
                hit=true
            fi
        done < "${SUITE_MAP}"
        $hit && continue

        # 4) Fallback: header/include affinity — a changed src file is picked
        #    up by suites whose test sources include it (by basename). Catches
        #    cross-module usage like dfm-framework tests including a
        #    src/dfm-base header that the COVERS prefix alone would miss.
        local base stem
        base="$(basename "$p")"
        stem="${base%.*}"
        local -a extra=()
        while IFS=$'\t' read -r suite dir; do
            [[ "$suite" == \#* || -z "$suite" ]] && continue
            # already selected via prefix match? then skip
            if [ -n "${SELECTION_REASON[$suite]+x}" ]; then
                case "${SELECTION_REASON[$suite]}" in *"$p"*) continue ;; esac
            fi
            if grep -rqs --include='*.cpp' --include='*.h' \
                   -E "#[[:space:]]*include[[:space:]]*[<\"][^\"]*/?${base}[>\"]" \
                   "${PROJECT_ROOT}/${dir}"; then
                extra+=("$suite")
            fi
        done < "${SUITE_MAP}"
        if [ ${#extra[@]} -gt 0 ]; then
            for suite in "${extra[@]}"; do
                select_suite "$suite" "includes ${base}"
            done
            echo "    mapped (include):   $p -> ${extra[*]}"
            continue
        fi

        # 5) No suite matched — policy decides
        echo "    NO suite matched:   $p"
        NO_MATCH_FILES+=("$p")
    done

    if [ "$matched_any_code" = false ]; then
        echo "==> All changed files are non-code (docs/packaging/assets), nothing to test."
        exit 0
    fi
}

# ---------------------------------------------------------------------------
# Configure / build / run
# ---------------------------------------------------------------------------
cached_coverage() { # current DFM_ENABLE_COVERAGE cache value
    grep -E '^DFM_ENABLE_COVERAGE(:BOOL)?=' "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null \
        | head -1 | grep -oE '(ON|OFF)$' || echo "OFF"
}

configure_project() {
    mkdir -p "${BUILD_DIR}"
    local -a args=(
        -S "${PROJECT_ROOT}"
        -B "${BUILD_DIR}"
        -DCMAKE_BUILD_TYPE=Debug
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DOPT_ENABLE_BUILD_UT=ON
    )
    if [ "${COVERAGE}" = true ]; then
        args+=(
            -DDFM_ENABLE_COVERAGE=ON
            -DCMAKE_CXX_FLAGS="--coverage -O0 -fno-inline"
            -DCMAKE_EXE_LINKER_FLAGS="--coverage"
        )
    fi
    cmake "${args[@]}"
}

build_targets() { # build_targets <target...>
    echo "==> Building targets: $*"
    cmake --build "${BUILD_DIR}" -j"$(nproc)" --target "$@"
}

run_ctest() { # run_ctest <regex>
    echo "==> Running tests (ctest -R '$1')..."
    ( cd "${BUILD_DIR}" && ctest -R "$1" --output-on-failure --timeout 120 )
}

coverage_report() {
    echo "==> Generating coverage report..."
    local COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${COVERAGE_DIR}"

    lcov --capture --directory "${BUILD_DIR}" --output-file "${COVERAGE_DIR}/coverage.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

    lcov --remove "${COVERAGE_DIR}/coverage.info" \
        '*/3rdparty/*' \
        '*/testutils/*' \
        '*/autotests/*' \
        '*/build*' \
        '/usr/*' \
        --output-file "${COVERAGE_DIR}/coverage-filtered.info" \
        --rc lcov_branch_coverage=1 2>/dev/null || true

    genhtml "${COVERAGE_DIR}/coverage-filtered.info" \
        --output-directory "${COVERAGE_DIR}/html" \
        --branch-coverage \
        --legend \
        --title "DDE File Manager - Unit Test Coverage" 2>/dev/null || true

    echo ""
    echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
    lcov --summary "${COVERAGE_DIR}/coverage-filtered.info" --rc lcov_branch_coverage=1 2>/dev/null || \
        echo "Warning: Could not generate coverage summary (lcov may not be installed)"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if [ "${INCREMENTAL}" = true ]; then
    # ----- incremental path -----
    if [ "${COVERAGE}" = true ] && [ -f "${BUILD_DIR}/CMakeCache.txt" ] \
           && [ "$(cached_coverage)" != "ON" ]; then
        echo "==> Coverage requested but build dir is not instrumented, reconfiguring..."
        rm -rf "${BUILD_DIR}"
    fi

    ensure_suite_map

    if [ -n "${MANUAL_SUITES}" ]; then
        # manual selection wins — for debugging the mapping itself
        IFS=$', \t\n' read -ra SELECTED <<< "${MANUAL_SUITES}"
        echo "==> Manual suite selection: ${SELECTED[*]}"
    else
        declare -a NO_MATCH_FILES=()
        map_changed_to_suites

        if [ ${#SELECTED[@]} -eq 0 ]; then
            case "${ON_NO_MATCH}" in
                skip)
                    echo "==> No suite matched changed files (--on-no-match=skip), done."
                    exit 0 ;;
                strict)
                    echo "==> No suite matched changed files (--on-no-match=strict)." >&2
                    exit 3 ;;
                *)
                    echo "==> No suite matched changed files, falling back to ALL suites (fail-safe)."
                    select_all_suites "no-match fallback" ;;
            esac
        fi
    fi

    # dedupe + sort for a stable ctest regex
    mapfile -t SELECTED < <(printf '%s\n' "${SELECTED[@]}" | sort -u)
    echo "==> Selected suites (${#SELECTED[@]}):"
    local_suite=""
    for local_suite in "${SELECTED[@]}"; do
        echo "    - ${local_suite}  [${SELECTION_REASON[${local_suite}]}]"
    done

    if [ "${LIST_ONLY}" = true ]; then
        echo "==> Dry run, nothing executed."
        exit 0
    fi

    if [ "${RUN_ONLY}" = false ]; then
        build_targets "${SELECTED[@]}"
    fi

    ctest_regex="^($(IFS='|'; echo "${SELECTED[*]}"))$"
    run_ctest "${ctest_regex}"

    [ "${COVERAGE}" = true ] && coverage_report
else
    # ----- full-run path (original behavior) -----
    if [ "${RUN_ONLY}" = false ]; then
        echo "==> [1/4] Configuring (fresh)..."
        rm -rf "${BUILD_DIR}"
        configure_project

        echo "==> [2/4] Building..."
        cmake --build "${BUILD_DIR}" -j"$(nproc)"
    fi

    echo "==> [3/4] Running all tests..."
    ( cd "${BUILD_DIR}" && ctest --output-on-failure --timeout 120 )

    if [ "${COVERAGE}" = true ]; then
        echo "==> [4/4] Generating coverage report..."
        coverage_report
    fi
fi

echo ""
echo "Done."
