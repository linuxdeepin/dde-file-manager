#!/bin/bash

# DDE文件管理器单元测试一键运行脚本
# 功能：编译单元测试 + 运行测试 + 生成覆盖率报告

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

# 获取目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "========================================"
echo "  DDE文件管理器单元测试一键运行工具"
echo "========================================"
echo "项目根目录: $PROJECT_ROOT"
echo "构建目录: $BUILD_DIR"

# 步骤1：清理和创建构建目录
print_step "准备构建环境..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
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

# 步骤4：运行测试
print_step "运行单元测试..."
if ctest --output-on-failure; then
    print_success "所有测试通过"
    TEST_PASSED=true
else
    print_error "部分测试失败"
    TEST_PASSED=false
fi

# 步骤5：生成覆盖率报告
print_step "生成覆盖率报告..."
if make coverage-all 2>/dev/null; then
    print_success "覆盖率报告生成完成"
    
    # 查找并显示覆盖率报告
    if [ -d "coverage-html" ]; then
        COVERAGE_PATH="$BUILD_DIR/coverage-html/index.html"
        print_success "📊 覆盖率报告: file://$COVERAGE_PATH"
    fi
else
    echo "⚠️  覆盖率生成失败，跳过"
fi

# 最终结果
echo "========================================"
if [ "$TEST_PASSED" = true ]; then
    print_success "🎉 单元测试执行完成！"
else
    print_error "❌ 单元测试有失败，请检查上面的错误信息"
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