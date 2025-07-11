# DDE文件管理器现代化单元测试架构

这是DDE文件管理器的单元测试基础设施，基于**零侵入自动文件发现**的设计理念。

## ✨ 核心特性

- **🚀 一键运行**：`./run-ut.sh` 完成编译+测试+覆盖率报告
- **🔍 零侵入**：完全不修改 `src/` 目录下的任何文件
- **🤖 自动发现**：自动扫描源文件，无需手动维护文件列表
- **📊 覆盖率收集**：自动生成HTML格式覆盖率报告
- **🔧 私有成员访问**：支持白盒测试，可访问私有成员
- **⚡ 高效构建**：使用CMake对象库，避免重复编译
- **🎯 统一目标**：每个组件对应一个测试目标，避免目标过多

## 🎯 解决的问题

| 传统测试架构问题 | 新架构解决方案 |
|---|---|
| ❌ 需要修改源码CMakeLists.txt | ✅ 完全不修改源码目录 |
| ❌ 手动维护源文件列表 | ✅ 自动扫描和包含源文件 |
| ❌ 复杂的依赖配置 | ✅ 根据组件名自动推导依赖 |
| ❌ 多个脚本难以使用 | ✅ 一个脚本完成所有操作 |
| ❌ 每个cpp文件一个target | ✅ 每个组件一个统一的测试target |

## 🏗️ 架构设计

### 目标映射关系

```
src/
├── dfm-framework/     →  autotests: dfm-framework-tests
├── dfm-base/         →  autotests: dfm-base-tests  
├── dfm-extension/    →  autotests: dfm-extension-tests
└── ...               →  autotests: ...-tests
```

### 统一测试目标

每个组件的所有 `test_*.cpp` 文件都会被合并到一个统一的测试目标中：

```cmake
# 例如：dfm-framework组件
dfm-framework-tests:
  ├── test_eventsequence.cpp
  ├── test_framelogmanager.cpp
  ├── test_listener.cpp
  ├── test_plugin.cpp
  └── ... (所有test_*.cpp文件)
```

这种设计的优势：
- **🎯 目标数量可控**：即使有1000个测试文件，也只有与组件数量相等的测试目标
- **🚀 构建效率高**：减少了CMake target的数量，提高构建速度
- **📊 覆盖率统一**：每个组件的覆盖率数据统一收集和报告
- **🔧 管理简单**：运行、调试、分析都以组件为单位

## 🚀 快速开始

### 一键运行（推荐）

```bash
cd autotests
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
cd autotests
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
autotests/
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

1. 进入组件测试目录：`autotests/units/dfm-framework/`
2. 创建测试文件：`test_yourfeature.cpp`
3. 文件会自动被发现并合并到 `dfm-framework-tests` 目标中

### 为新组件添加测试

1. 创建组件目录：`mkdir autotests/units/your-component/`
2. 创建CMakeLists.txt：
   ```cmake
   # 仅需一行配置！
   dfm_create_component_test(your-component)
   ```
3. 添加测试文件：`test_*.cpp`
4. 系统会自动创建 `your-component-tests` 目标

## 📊 覆盖率报告

运行测试后，覆盖率报告位置：
- **所有组件汇总**：`autotests/build/coverage/html/index.html`
- **单个组件**：`autotests/build/coverage/{组件名}/html/index.html`

查看方法：
```bash
# 在浏览器中打开
firefox autotests/build/coverage/html/index.html
```

## 🔧 高级用法

### 运行特定组件的测试

```bash
cd autotests/build
# 运行dfm-framework组件的测试
ctest -L dfm-framework --output-on-failure

# 或者直接运行测试目标
./tests/dfm-framework/dfm-framework-tests
```

### 生成特定组件的覆盖率

```bash
cd autotests/build
make coverage-dfm-framework
```

### 查看所有可用的Make目标

```bash
cd autotests/build
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

3. **统一测试目标**：
   ```cmake
   add_executable(${COMPONENT}-tests ${ALL_TEST_FILES} $<TARGET_OBJECTS:${COMPONENT}-test-objects>)
   ```

4. **覆盖率编译选项**：
   ```cmake
   target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
   ```

### 支持的组件

当前支持的组件（自动检测`src/`目录）：
- `dfm-framework`: 框架组件 → `dfm-framework-tests`
- `dfm-base`: 基础组件 → `dfm-base-tests`
- `dfm-extension`: 扩展组件 → `dfm-extension-tests`

### 测试目标命名规则

- 组件源码：`src/{component}/`
- 测试目录：`autotests/units/{component}/`
- 测试目标：`{component}-tests`
- 对象库：`{component}-test-objects`

## 🐛 故障排除

### 常见问题

**Q: 覆盖率报告不生成**  
A: 安装lcov：`sudo apt-get install lcov`

**Q: 测试目标过多**  
A: 新架构已解决，每个组件只有一个测试目标

**Q: 如何运行单个测试文件**  
A: 使用GTest的过滤功能：`./dfm-framework-tests --gtest_filter="YourTestClass*"`

### 调试模式

启用详细输出：
```bash
cd autotests/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
make VERBOSE=1
```

## 🎯 性能优势

与旧架构相比：
- **目标数量**：从 N个测试文件 → 1个组件目标
- **构建速度**：减少CMake处理时间
- **内存使用**：减少并行构建时的内存占用
- **管理复杂度**：O(组件数) 而非 O(测试文件数)

