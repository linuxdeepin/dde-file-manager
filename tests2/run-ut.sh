#!/bin/bash

# DDE文件管理器单元测试一键运行脚本
# 功能：编译单元测试 + 运行测试 + 生成覆盖率报告 + 生成测试报表

set -e  # 错误时退出

# 颜色输出
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

# 获取目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
REPORT_DIR="$BUILD_DIR/test-reports"

echo "========================================"
echo "  DDE文件管理器单元测试一键运行工具"
echo "========================================"
echo "项目根目录: $PROJECT_ROOT"
echo "构建目录: $BUILD_DIR"
echo "报告目录: $REPORT_DIR"

# 步骤1：清理和创建构建目录
print_step "准备构建环境..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
mkdir -p "$REPORT_DIR"
cd "$BUILD_DIR"

# 步骤2：CMake配置
print_step "配置CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DDFM_SOURCE_DIR="$PROJECT_ROOT" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

print_success "CMake配置完成"

# 步骤3：编译
print_step "编译测试..."
make -j$(nproc)
print_success "编译完成"

# 步骤4：运行测试并捕获结果
print_step "运行单元测试..."
TEST_START_TIME=$(date +%s)

# 运行测试并将结果保存到文件
if ctest --output-on-failure --verbose > "$REPORT_DIR/test_output.log" 2>&1; then
    print_success "所有测试通过"
    TEST_PASSED=true
    TEST_EXIT_CODE=0
else
    print_error "部分测试失败"
    TEST_PASSED=false
    TEST_EXIT_CODE=$?
fi

TEST_END_TIME=$(date +%s)
TEST_DURATION=$((TEST_END_TIME - TEST_START_TIME))

# 生成XML格式的测试结果
ctest --output-junit "$REPORT_DIR/test_results.xml" >/dev/null 2>&1 || true

# 步骤5：生成覆盖率报告（无论测试是否失败都执行）
print_step "生成覆盖率报告..."
COVERAGE_START_TIME=$(date +%s)

# 强制生成覆盖率报告，忽略错误
set +e  # 临时禁用错误退出

# 先尝试收集覆盖率数据，即使测试失败也要收集
print_step "收集覆盖率数据..."
mkdir -p coverage
lcov --directory . --capture --output-file coverage/total.info > "$REPORT_DIR/coverage_output.log" 2>&1 || true
lcov --extract coverage/total.info "*/src/*" --output-file coverage/filtered.info >> "$REPORT_DIR/coverage_output.log" 2>&1 || true

# 如果有覆盖率数据，生成HTML报告
if [ -f "coverage/filtered.info" ] && [ -s "coverage/filtered.info" ]; then
    genhtml --output-directory coverage/html --title "DFM Coverage Report" --show-details --legend coverage/filtered.info >> "$REPORT_DIR/coverage_output.log" 2>&1
    if [ $? -eq 0 ]; then
        print_success "覆盖率报告生成完成"
        COVERAGE_SUCCESS=true
        COVERAGE_PATH="$BUILD_DIR/coverage/html/index.html"
        print_success "📊 覆盖率报告: file://$COVERAGE_PATH"
    else
        print_warning "HTML覆盖率报告生成失败，但数据已收集"
        COVERAGE_SUCCESS=false
    fi
else
    print_warning "没有收集到覆盖率数据"
    COVERAGE_SUCCESS=false
fi

set -e  # 重新启用错误退出

COVERAGE_END_TIME=$(date +%s)
COVERAGE_DURATION=$((COVERAGE_END_TIME - COVERAGE_START_TIME))

# 步骤6：生成综合测试报表
print_step "生成测试报表..."
python3 "$SCRIPT_DIR/generate_report.py" \
    --build-dir "$BUILD_DIR" \
    --report-dir "$REPORT_DIR" \
    --test-passed "$TEST_PASSED" \
    --test-duration "$TEST_DURATION" \
    --coverage-success "$COVERAGE_SUCCESS" \
    --coverage-duration "$COVERAGE_DURATION" \
    --project-root "$PROJECT_ROOT"

if [ $? -eq 0 ]; then
    print_success "测试报表生成完成"
    REPORT_PATH="$REPORT_DIR/test_report.html"
    print_success "📈 测试报表: file://$REPORT_PATH"
else
    print_warning "测试报表生成失败"
fi

# 最终结果
echo "========================================"
if [ "$TEST_PASSED" = true ]; then
    print_success "🎉 单元测试执行完成！"
else
    print_error "❌ 单元测试有失败，请检查测试报表"
fi

echo ""
echo "📋 生成的报告："
echo "  测试报表: file://$REPORT_DIR/test_report.html"
if [ "$COVERAGE_SUCCESS" = true ]; then
    echo "  覆盖率报告: file://$BUILD_DIR/coverage/html/index.html"
fi
echo ""
echo "快速命令："
echo "  重新运行测试: cd $BUILD_DIR && ctest"
echo "  重新生成覆盖率: cd $BUILD_DIR && make coverage-all"
echo "  查看可用目标: cd $BUILD_DIR && make help"
echo "========================================"

# 如果测试失败，返回非零退出码
if [ "$TEST_PASSED" != true ]; then
    exit 1
fi 