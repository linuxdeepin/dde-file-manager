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
    echo -e "${BLUE}[STEP]${NC} $1"
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

# Get directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
REPORT_DIR="$BUILD_DIR/test-reports"

echo "========================================"
echo "  DDE File Manager Unit Test Runner"
echo "========================================"
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"
echo "Report directory: $REPORT_DIR"

# Step 1: Clean and create build directory
print_step "Preparing build environment..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
mkdir -p "$REPORT_DIR"

# Step 2: CMake configuration
print_step "Configuring CMake..."
cd "$BUILD_DIR"
cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDFM_STANDALONE_TEST=ON
print_success "CMake configuration completed"

# Step 3: Compile
print_step "Compiling tests..."
cmake --build . -j $(nproc)
print_success "Compilation completed"

# Step 4: Run tests and capture results
print_step "Running unit tests..."
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

# Step 5: Generate coverage report (execute regardless of test results)
print_step "Generating coverage report..."
COVERAGE_START_TIME=$(date +%s)

# Temporarily disable error exit
set +e

# Try to collect coverage data even if tests failed
print_step "Collecting coverage data..."
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

# Step 6: Generate comprehensive test report
print_step "Generating test report..."
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

# Final results
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
echo "========================================"

# Return non-zero exit code if tests failed
if [ "$TEST_PASSED" != true ]; then
    exit 1
fi 