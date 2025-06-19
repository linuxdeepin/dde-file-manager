# DDE文件管理器现代化单元测试架构 (tests2)

这是DDE文件管理器的现代化单元测试基础设施，基于**零侵入自动文件发现**的设计理念。

## ✨ 核心特性

- **🚀 一键运行**：`./run-ut.sh` 完成编译+测试+覆盖率报告
- **🔍 零侵入**：完全不修改 `src/` 目录下的任何文件
- **🤖 自动发现**：自动扫描源文件，无需手动维护文件列表
- **📊 覆盖率收集**：自动生成HTML格式覆盖率报告
- **🔧 私有成员访问**：支持白盒测试，可访问私有成员
- **⚡ 高效构建**：使用CMake对象库，避免重复编译

## 🎯 解决的问题

| 传统测试架构问题 | 新架构解决方案 |
|---|---|
| ❌ 需要修改源码CMakeLists.txt | ✅ 完全不修改源码目录 |
| ❌ 手动维护源文件列表 | ✅ 自动扫描和包含源文件 |
| ❌ 复杂的依赖配置 | ✅ 根据组件名自动推导依赖 |
| ❌ 多个脚本难以使用 | ✅ 一个脚本完成所有操作 |

## 🚀 快速开始

### 一键运行（推荐）

```bash
cd tests2
./run-ut.sh
```

这个脚本会自动完成：
1. 清理旧的构建产物
2. 配置CMake
3. 编译所有测试
4. 运行单元测试
5. 生成覆盖率报告

### 手动运行（高级用户）

```bash
cd tests2
mkdir build && cd build

# 配置
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 编译
make -j$(nproc)

# 运行测试
ctest --output-on-failure

# 生成覆盖率
make coverage-all
```

## 📂 目录结构

```
tests2/
├── run-ut.sh                   # 🚀 一键运行脚本
├── CMakeLists.txt              # 根配置文件
├── cmake/                      # CMake工具模块
│   ├── DFMTestUtils.cmake      # 核心：自动文件发现
│   ├── TestDiscovery.cmake     # 测试发现系统  
│   └── CoverageTargets.cmake   # 覆盖率目标管理
├── framework/                  # 测试框架核心
│   ├── dfm-test-base.h        # 增强测试工具
│   └── test-main.cpp          # 通用测试入口
├── units/                      # 单元测试（按组件组织）
│   ├── dfm-framework/         # 框架组件测试
│   ├── dfm-base/              # 基础组件测试
│   └── dfm-extension/         # 扩展组件测试
└── fixtures/                  # 测试数据和固件
```

## 🧪 添加新测试

### 为现有组件添加测试

1. 进入组件测试目录：`tests2/units/dfm-framework/`
2. 创建测试文件：`test_yourfeature.cpp`
3. 文件会自动被发现和编译

### 为新组件添加测试

1. 创建组件目录：`mkdir tests2/units/your-component/`
2. 创建CMakeLists.txt：
   ```cmake
   # 仅需一行配置！
   dfm_create_component_test(your-component)
   ```
3. 添加测试文件：`test_*.cpp`

## 📊 覆盖率报告

运行测试后，覆盖率报告位置：
- **所有组件汇总**：`tests2/build/coverage/combined/index.html`
- **单个组件**：`tests2/build/coverage/{组件名}/index.html`

查看方法：
```bash
# 在浏览器中打开
firefox tests2/build/coverage/combined/index.html
```

## 🔧 高级用法

### 运行特定组件的测试

```bash
cd tests2/build
ctest -L dfm-framework --output-on-failure
```

### 生成特定组件的覆盖率

```bash
cd tests2/build
make coverage-dfm-framework
```

### 查看所有可用的Make目标

```bash
cd tests2/build
make help
```

## 🛠️ 技术细节

### 核心原理

1. **自动文件发现**：
   ```cmake
   file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/${COMPONENT}/*.cpp")
   ```

2. **对象库技术**：
   ```cmake
   add_library(${COMPONENT}-test-objects OBJECT ${SOURCES})
   ```

3. **覆盖率编译选项**：
   ```cmake
   target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
   ```

### 支持的组件

当前支持的组件（自动检测`src/`目录）：
- `dfm-framework`: 框架组件
- `dfm-base`: 基础组件  
- `dfm-extension`: 扩展组件

## 🐛 故障排除

### 常见问题

**Q: 运行`./run-ut.sh`提示权限错误**  
A: 添加执行权限：`chmod +x run-ut.sh`

**Q: CMake配置失败，找不到Qt6**  
A: 安装Qt6开发包：`sudo apt-get install qt6-base-dev qt6-tools-dev`

**Q: 找不到GTest**  
A: 安装GTest：`sudo apt-get install libgtest-dev`

**Q: 覆盖率报告不生成**  
A: 安装lcov：`sudo apt-get install lcov`

**Q: 测试编译失败，找不到头文件**  
A: 检查`PROJECT_SOURCE_DIR`是否正确设置

### 调试模式

启用详细输出：
```bash
cd tests2/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
make VERBOSE=1
```

## 📋 系统要求

- **CMake**: >= 3.16
- **编译器**: GCC/Clang 支持C++17
- **Qt**: Qt6 (Core, Widgets, Test)
- **测试框架**: Google Test
- **覆盖率工具**: lcov, genhtml
- **系统**: Linux (Ubuntu/Debian 推荐)

## 📚 相关文档

- [蓝图设计文档](../ut-blueprint-zh-cn.md)
- [开发计划](../ut-plan.md)
- [任务完成报告](TASK004-COMPLETION-REPORT.md)

---

**🎉 现代化测试的力量：一行配置，自动化一切！**