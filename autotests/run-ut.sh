#!/bin/bash

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# DDE File Manager Unit Test Runner Script
# Function: Compile unit tests + Run tests + Generate coverage report + Generate test report

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${BLUE}[STEP $1]${NC} $2"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --from-step <N>    Start execution from step N (1-6)"
    echo "  -h, --help         Show this help message"
    echo ""
    echo "Steps:"
    echo "  1. Prepare build environment"
    echo "  2. Configure CMake"
    echo "  3. Compile tests"
    echo "  4. Run unit tests"
    echo "  5. Generate coverage report"
    echo "  6. Generate test report"
    echo ""
    echo "Examples:"
    echo "  $0                 # Run all steps"
    echo "  $0 --from-step 4   # Start from running tests (skip build)"
    echo "  $0 --from-step 5   # Start from coverage generation"
}

# Parse command line arguments
START_STEP=1
while [[ $# -gt 0 ]]; do
    case $1 in
        --from-step)
            START_STEP="$2"
            if ! [[ "$START_STEP" =~ ^[1-6]$ ]]; then
                print_error "Invalid step number: $START_STEP. Must be 1-6."
                exit 1
            fi
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Get directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-autotests"
REPORT_DIR="$BUILD_DIR/test-reports"

echo "========================================"
echo "  DDE File Manager Unit Test Runner"
echo "========================================"
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"
echo "Report directory: $REPORT_DIR"
if [ "$START_STEP" -gt 1 ]; then
    print_info "Starting from step $START_STEP"
fi
echo ""

# Initialize variables that might be used by later steps
TEST_PASSED=false
TEST_DURATION=0
COVERAGE_SUCCESS=false
COVERAGE_DURATION=0

step_1_prepare_build_env() {
    print_step 1 "Preparing build environment..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    mkdir -p "$BUILD_DIR"
    mkdir -p "$REPORT_DIR"
    print_success "Build environment prepared"
}

step_2_configure_cmake() {
    print_step 2 "Configuring CMake..."
    cd "$BUILD_DIR"
    cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDFM_STANDALONE_TEST=ON
    print_success "CMake configuration completed"
}

step_3_compile_tests() {
    print_step 3 "Compiling tests..."
    cd "$BUILD_DIR"
    cmake --build . -j $(nproc)
    print_success "Compilation completed"
}

step_4_run_tests() {
    print_step 4 "Running unit tests..."
    
    # Ensure we have the report directory
    mkdir -p "$REPORT_DIR"
    
    TEST_START_TIME=$(date +%s)

    # Run tests and save results to file
    cd "$BUILD_DIR"
    if ctest --output-on-failure --verbose > "$REPORT_DIR/test_output.log" 2>&1; then
        print_success "All tests passed"
        TEST_PASSED=true
        TEST_EXIT_CODE=0
    else
        print_error "Some tests failed"
        TEST_PASSED=false
        TEST_EXIT_CODE=$?
    fi

    TEST_END_TIME=$(date +%s)
    TEST_DURATION=$((TEST_END_TIME - TEST_START_TIME))

    # Generate XML format test results
    ctest --output-junit "$REPORT_DIR/test_results.xml" >/dev/null 2>&1 || true
    
    print_info "Test execution completed in ${TEST_DURATION}s"
}

step_5_generate_coverage() {
    print_step 5 "Generating coverage report..."
    COVERAGE_START_TIME=$(date +%s)

    # Temporarily disable error exit
    set +e

    # Try to collect coverage data even if tests failed
    print_info "Collecting coverage data..."
    mkdir -p "$BUILD_DIR/coverage"
    lcov --directory "$BUILD_DIR" --capture --output-file "$BUILD_DIR/coverage/total.info" > "$REPORT_DIR/coverage_output.log" 2>&1 || true

    # Filter coverage data to only include source files
    if [ -f "$BUILD_DIR/coverage/total.info" ]; then
        lcov --extract "$BUILD_DIR/coverage/total.info" "*/src/*" --output-file "$BUILD_DIR/coverage/filtered.info" >> "$REPORT_DIR/coverage_output.log" 2>&1 || true
        # Remove test files from coverage
        lcov --remove "$BUILD_DIR/coverage/filtered.info" "*/test*" "*/autotests/*" --output-file "$BUILD_DIR/coverage/filtered.info" >> "$REPORT_DIR/coverage_output.log" 2>&1 || true
    fi

    # Generate HTML report if we have coverage data
    if [ -f "$BUILD_DIR/coverage/filtered.info" ] && [ -s "$BUILD_DIR/coverage/filtered.info" ]; then
        genhtml --output-directory "$BUILD_DIR/coverage/html" --title "DFM Coverage Report" --show-details --legend "$BUILD_DIR/coverage/filtered.info" >> "$REPORT_DIR/coverage_output.log" 2>&1
        if [ $? -eq 0 ]; then
            print_success "Coverage report generated successfully"
            COVERAGE_SUCCESS=true
            COVERAGE_PATH="$BUILD_DIR/coverage/html/index.html"
            print_success "📊 Coverage report: file://$COVERAGE_PATH"
        else
            print_warning "HTML coverage report generation failed, but data was collected"
            COVERAGE_SUCCESS=false
        fi
    else
        print_warning "No coverage data collected"
        COVERAGE_SUCCESS=false
    fi

    # Re-enable error exit
    set -e

    COVERAGE_END_TIME=$(date +%s)
    COVERAGE_DURATION=$((COVERAGE_END_TIME - COVERAGE_START_TIME))
    
    print_info "Coverage generation completed in ${COVERAGE_DURATION}s"
}

step_6_generate_test_report() {
    print_step 6 "Generating comprehensive test report..."
    
    # Load test results if starting from this step
    if [ "$START_STEP" -eq 6 ]; then
        print_info "Loading previous test results..."
        
        # Try to determine test status from existing files
        if [ -f "$REPORT_DIR/test_output.log" ]; then
            if grep -q "100% tests passed" "$REPORT_DIR/test_output.log"; then
                TEST_PASSED=true
            else
                TEST_PASSED=false
            fi
            
            # Try to extract duration from log (fallback to 0 if not found)
            TEST_DURATION=$(grep -o "Total Test time.*: [0-9]*" "$REPORT_DIR/test_output.log" | grep -o "[0-9]*" || echo "0")
        else
            print_warning "No previous test results found, using defaults"
            TEST_PASSED=false
            TEST_DURATION=0
        fi
        
        # Check if coverage was successful
        if [ -f "$BUILD_DIR/coverage/html/index.html" ]; then
            COVERAGE_SUCCESS=true
        else
            COVERAGE_SUCCESS=false
        fi
        COVERAGE_DURATION=0
        
        print_info "Loaded: TEST_PASSED=$TEST_PASSED, COVERAGE_SUCCESS=$COVERAGE_SUCCESS"
    fi
    
    python3 "$SCRIPT_DIR/generate-report.py" \
        --build-dir "$BUILD_DIR" \
        --report-dir "$REPORT_DIR" \
        --test-passed "$TEST_PASSED" \
        --test-duration "$TEST_DURATION" \
        --coverage-success "$COVERAGE_SUCCESS" \
        --coverage-duration "$COVERAGE_DURATION" \
        --project-root "$PROJECT_ROOT"

    if [ $? -eq 0 ]; then
        print_success "Test report generated successfully"
        REPORT_PATH="$REPORT_DIR/test_report.html"
        print_success "📈 Test report: file://$REPORT_PATH"
    else
        print_warning "Test report generation failed"
    fi
}

# Execute steps based on START_STEP
case $START_STEP in
    1)
        step_1_prepare_build_env
        step_2_configure_cmake
        step_3_compile_tests
        step_4_run_tests
        step_5_generate_coverage
        step_6_generate_test_report
        ;;
    2)
        # Ensure build directory exists
        mkdir -p "$BUILD_DIR"
        mkdir -p "$REPORT_DIR"
        step_2_configure_cmake
        step_3_compile_tests
        step_4_run_tests
        step_5_generate_coverage
        step_6_generate_test_report
        ;;
    3)
        # Ensure build directory exists
        mkdir -p "$BUILD_DIR"
        mkdir -p "$REPORT_DIR"
        step_3_compile_tests
        step_4_run_tests
        step_5_generate_coverage
        step_6_generate_test_report
        ;;
    4)
        # Ensure build directory exists
        mkdir -p "$REPORT_DIR"
        step_4_run_tests
        step_5_generate_coverage
        step_6_generate_test_report
        ;;
    5)
        # Ensure build directory exists
        mkdir -p "$REPORT_DIR"
        step_5_generate_coverage
        step_6_generate_test_report
        ;;
    6)
        # Ensure build directory exists
        mkdir -p "$REPORT_DIR"
        step_6_generate_test_report
        ;;
esac

# Final results
echo ""
echo "========================================"
if [ "$TEST_PASSED" = true ]; then
    print_success "🎉 Unit test execution completed!"
else
    print_error "❌ Unit tests have failures, please check the test report"
fi

echo ""
echo "📋 Generated reports:"
echo "  Test report: file://$REPORT_DIR/test_report.html"
if [ "$COVERAGE_SUCCESS" = true ]; then
    echo "  Coverage report: file://$BUILD_DIR/coverage/html/index.html"
fi
echo ""
echo "Quick commands:"
echo "  Re-run tests: cd $BUILD_DIR && ctest"
echo "  Re-generate coverage: cd $BUILD_DIR && make coverage-all"
echo "  View available targets: cd $BUILD_DIR && make help"
echo "  Run from specific step: $0 --from-step <N>"
echo "========================================"

# Return non-zero exit code if tests failed
if [ "$TEST_PASSED" != true ]; then
    exit 1
fi 