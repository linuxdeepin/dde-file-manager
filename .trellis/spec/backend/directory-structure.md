# 后端目录结构规范

> DDE File Manager 后端代码组织规范。

---

## 概述

项目采用模块化分层架构，源代码位于 `src/` 目录下，按功能域划分为独立的模块。

---

## 源代码根目录结构

```
src/
├── dfm-base/           # 基础库 (核心框架)
├── dfm-framework/      # 框架库 (应用框架)
├── dfm-extension/      # 扩展库 (插件接口)
├── plugins/            # 插件 (功能模块)
├── services/           # 后台服务
├── apps/               # 应用程序
├── tools/              # 工具程序
├── config/             # 配置文件
└── external/           # 外部依赖
```

---

## 各模块详细说明

### dfm-base (基础库)

**职责**: 提供跨框架的核心功能，包括文件操作、数据库、配置管理等。

**目录结构**:
```
dfm-base/
├── base/
│   ├── application/    # 应用程序框架 (Settings, Application)
│   ├── db/            # 数据库层 (SQLite ORM, 连接池)
│   ├── device/        # 设备管理
│   ├── file/          # 文件操作工具
│   ├── event/         # 事件系统
│   └── utils/         # 通用工具 (FinallyUtil, 等)
├── interfaces/        # 接口定义
├── widgets/           # 自定义组件
├── qrc/              # 资源文件
└── CMakeLists.txt    # 构建配置
```

**示例文件**:
- `src/dfm-base/base/db/sqliteconnectionpool.cpp` - 数据库连接池
- `src/dfm-base/base/application/settings.cpp` - 配置管理
- `include/dfm-base/utils/finallyutil.h` - 资源清理工具

---

### dfm-framework (框架库)

**职责**: 提供 DDE 文件管理器的框架级功能，包括视图、模型、控制器等。

**目录结构**:
```
dfm-framework/
├── controller/        # 控制器层
├── event/            # 框架事件
├── interfaces/       # 框架接口
├── models/           # 模型层
├── views/            # 视图层
└── widget/           # 框架组件
```

---

### plugins (插件)

**职责**: 按功能域划分的独立功能模块，每个插件实现特定功能。

**插件分类**:
```
plugins/
├── common/          # 共享插件 (跨平台使用)
│   ├── dfmplugin-bookmark/      # 书签功能
│   ├── dfmplugin-menu/          # 右键菜单
│   └── dfmplugin-fileoperations/# 文件操作
├── daemon/          # 后台守护插件
│   └── tag/                      # 文件标签服务
├── filemanager/     # 文件管理器插件
│   ├── dfmplugin-workspace/     # 工作区视图
│   ├── dfmplugin-computer/      # 计算机/设备视图
│   └── dfmplugin-sidebar/       # 侧边栏
└── desktop/         # 桌面插件
    ├── ddplugin-core/           # 桌面核心
    ├── ddplugin-canvas/         # 桌面画布
    └── ddplugin-wallpapersetting/ # 壁纸设置
```

**插件命名规范**:
- 共享插件: `dfmplugin-*`
- 桌面插件: `ddplugin-*`
- 守护插件: 直接用功能名 (如 `tag`)

**插件内部结构**:
```
dfmplugin-workspace/
├── views/           # 视图类
├── models/          # 模型类
├── widgets/         # 组件类
├── utils/           # 辅助函数
├── private/         # 私有类 (D-Pointer 模式)
└── workspace.cpp    # 插件入口 (命名与插件名相关)
```

**注意**: 插件入口文件命名与插件主类相关，如书签插件使用 `bookmark.cpp`，工作区插件使用 `workspace.cpp`。

**示例文件**:
- `src/plugins/common/dfmplugin-bookmark/bookmark.cpp` - 书签插件入口
- `src/plugins/desktop/ddplugin-core/screen/screenqt.cpp` - 桌面屏幕管理

---

### services (后台服务)

**职责**: 独立运行的后台服务，处理耗时操作和系统级功能。

**目录结构**:
```
services/
├── accesscontrol/   # 访问控制服务
├── diskencrypt/     # 磁盘加密服务
├── mountcontrol/    # 挂载控制服务
├── sharecontrol/    # 共享控制服务
└── textindex/       # 文本索引服务
```

**服务内部结构**:
```
diskencrypt/
├── core/            # 核心逻辑
├── workers/         # 工作线程
├── helpers/         # 辅助类
├── dbus/           # DBus 接口
└── plugin.cpp      # 服务入口
```

**示例文件**:
- `src/services/diskencrypt/helpers/commonhelper.cpp` - 加密辅助工具
- `src/services/textindex/utils/pathexcludematcher.cpp` - 路径匹配工具

---

### apps (应用程序)

**职责**: 可执行应用程序，独立运行的程序。

**目录结构**:
```
apps/
├── dde-file-manager-preview/   # 文件预览程序
│   ├── filepreview/           # 主程序
│   ├── libdfm-preview/        # 预览库
│   └── pluginpreviews/        # 预览插件
└── ...
```

**预览插件分类**:
```
pluginpreviews/
├── image-preview/      # 图片预览
├── music-preview/      # 音乐预览
├── pdf-preview/        # PDF 预览
├── markdown-preview/   # Markdown 预览
└── dciicon-preview/    # DCI 图标预览
```

---

### tools (工具程序)

**职责**: 命令行工具或辅助工具。

---

### config (配置文件)

**职责**: 编译时配置和资源。

---

## 文件命名规范

### 目录命名
- 使用小写字母和连字符: `dfm-plugin-bookmark`, `ddplugin-core`
- 模块目录: 单词小写: `base`, `device`, `utils`

### 文件命名
- 头文件/源文件: 小写加下划线: `sqliteconnectionpool.h`, `finallyutil.h`
- 私有类: `private/` 子目录, 后缀 `_p.h`: `fileselectionmodel_p.h`

## 包含路径规范

| 组件 | 包含路径 | 命名空间 |
|------|----------|----------|
| dfm-base | `include/dfm-base/` | `dfmbase` |
| dfm-framework | `include/dfm-framework/` | `dfmframework` |
| dfm-extension | `include/dfm-extension/` | `dfmextension` |

## 架构分层

```
┌─────────────────────────────────────┐
│           applications               │  apps/ (应用入口)
├─────────────────────────────────────┤
│            plugins                  │  plugins/ (功能插件)
├─────────────────────────────────────┤
│           services                  │  services/ (后台服务)
├─────────────────────────────────────┤
│         dfm-framework              │  dfm-framework/ (框架层)
├─────────────────────────────────────┤
│          dfm-base                  │  dfm-base/ (基础层)
├─────────────────────────────────────┤
│            Qt / DTK                 │  外部依赖
└─────────────────────────────────────┘
```

**依赖原则**: 上层依赖下层，下层不依赖上层。

---

## 代码组织模式

### 1. Model-View-Controller (MVC)

插件层采用 MVC 模式:
- `models/` - 数据模型
- `views/` - 视图组件
- `utils/` - 业务逻辑 (控制器)

### 2. D-Pointer 模式 (私有实现)

私有类放在 `private/` 目录:
```
dfmplugin-workspace/
├── fileselectionmodel.h
├── fileselectionmodel.cpp
└── private/
    └── fileselectionmodel_p.h   # 私有实现
```

### 3. 模块化设计

每个服务/插件都是独立的子模块:
- 有自己的 `CMakeLists.txt`
- 有统一的 `plugin.cpp` 入口
- 可独立编译和发布

---

## 最佳实践

### Do
- 按功能域划分插件 (common/daemon/filemanager/desktop)
- 共享代码放 `dfm-base/`
- 私有实现用 `private/` 子目录
- 遵循命名规范 (小写+下划线)

### Don't
- 不要跨模块依赖 (如 daemon 依赖 filemanager)
- 不要在插件中直接实现基础功能
- 不要硬编码路径和配置

---

## 参考文档

- [CMake 约定](cmake-conventions.md) - 构建系统配置
- [架构设计](architecture-design.md) - 系统架构设计
- [插件架构](plugin-architecture.md) - 插件开发规范
