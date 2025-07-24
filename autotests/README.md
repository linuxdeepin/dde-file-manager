# DDE File Manager 单元测试

本目录包含 DDE 文件管理器的完整单元测试套件，支持测试运行、覆盖率分析和自动化报告生成。

## 依赖安装

### 必需依赖

```bash
sudo apt install lcov cmake python3
```

### Python 依赖

测试报告生成器需要 Python 3.6+ 环境，无需额外的 Python 包。

## 项目结构

```
autotests/
├── CMakeLists.txt           # 主构建配置文件
├── run-ut.sh               # 一键测试执行脚本
├── generate-report.py      # 测试报告生成脚本
├── report_generator/       # 报告生成器模块
├── libs/                   # 库单元测试
│   ├── dfm-base/          # dfm-base 库测试
│   ├── dfm-extension/     # dfm-extension 库测试
│   └── dfm-framework/     # dfm-framework 库测试
├── plugins/               # 插件单元测试
│   ├── dfmplugin-*/       # 各插件测试目录
│   └── ddplugin-*/        # 桌面插件测试目录
├── services/              # 服务单元测试
│   ├── accesscontrol/     # 访问控制服务测试
│   ├── diskencrypt/       # 磁盘加密服务测试
│   └── mountcontrol/      # 挂载控制服务测试
└── tools/                 # 工具单元测试
    └── upgrade/           # 升级工具测试
```

## 使用方法

### 快速开始

```bash
# 执行完整测试流程（推荐）
./run-ut.sh

# 查看帮助信息
./run-ut.sh --help
```

### 分步执行

脚本支持从指定步骤开始执行，适用于调试或重新生成报告：

```bash
# 从步骤 4 开始（跳过编译，直接运行测试）
./run-ut.sh --from-step 4

# 从步骤 5 开始（只生成覆盖率报告）
./run-ut.sh --from-step 5

# 从步骤 6 开始（只生成测试报告）
./run-ut.sh --from-step 6
```

### 执行步骤说明

1. **准备构建环境** - 清理并创建构建目录
2. **配置 CMake** - 生成构建文件
3. **编译测试** - 编译所有单元测试
4. **运行单元测试** - 执行测试并收集结果
5. **生成覆盖率报告** - 使用 lcov 生成代码覆盖率
6. **生成测试报告** - 生成综合 HTML 报告

## 输出文件

执行完成后，将在以下位置生成文件：

```
../build-autotests/
├── test-reports/
│   ├── test_report.html      # 综合测试报告
│   ├── test_output.log       # 测试输出日志
│   ├── test_results.xml      # JUnit 格式测试结果
│   └── coverage_output.log   # 覆盖率生成日志
└── coverage/
    └── html/
        └── index.html        # 代码覆盖率报告
```

## 手动执行测试

如需手动控制测试过程：

```bash
# 创建构建目录
mkdir -p ../build-autotests
cd ../build-autotests

# 配置 CMake
cmake ../autotests -DCMAKE_BUILD_TYPE=Debug -DDFM_STANDALONE_TEST=ON

# 编译
cmake --build . -j $(nproc)

# 运行测试
ctest --output-on-failure --verbose

# 生成覆盖率
lcov --directory . --capture --output-file coverage.info
genhtml coverage.info --output-directory coverage-html
```

## 故障排除

### 常见问题

1. **lcov 未安装**
   ```bash
   sudo apt install lcov
   ```

2. **编译错误**
   - 检查是否安装了所有必需的开发依赖
   - 确保文管开发环境正确配置

3. **权限问题**
   ```bash
   chmod +x run-ut.sh
   ```

4. **Python 脚本问题**
   - 确保系统已安装 Python 3.6+
   - 检查 python3 命令是否可用

### 调试选项

```bash
# 查看详细的 CMake 配置信息
cmake ../autotests -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON

# 查看可用的测试目标
cd ../build-autotests && make help

# 运行特定测试
cd ../build-autotests && ctest -R "测试名称"
```

## 开发者注意事项

- 测试使用 CMake + CTest 框架
- 支持独立构建模式（`DFM_STANDALONE_TEST=ON`）
- 自动生成 JUnit 格式的测试结果，便于 CI/CD 集成
- 覆盖率报告会自动过滤测试文件，只显示源代码覆盖率
- 报告生成器采用模块化设计，易于扩展和维护
