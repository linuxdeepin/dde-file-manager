# DDE 文件管理器扩展机制规范

## 概述

DDE 文件管理器（dde-file-manager）提供了一套完整的扩展机制，允许第三方开发者通过标准化的接口扩展文件管理器的功能。本规范定义了支持的扩展类型、实现方式、接口约定以及最佳实践。

## 扩展类型

DDE 文件管理器目前支持以下扩展类型：

### 1. 自定义菜单扩展

通过 Desktop Entry 文件向文件管理器的右键菜单添加自定义操作。提供两种实现方式：

- **Custom Menu Script（推荐）**：新版菜单扩展机制，提供更完善的接口和更好的兼容性
  - 详细规范：[custom-menu-script.md](01-custom-menu-script.md)
- **OEM Menu Script（已弃用）**：旧版菜单扩展机制，保留用于兼容性
  - 详细规范：[oem-menu-script.md](02-oem-menu-script.md)

### 2. 文件预览插件

为特定文件类型提供空格键快速预览功能。预览插件以独立进程方式运行，通过 D-Bus 与文件管理器通信。

- 详细规范：[dde-file-manager-preview-plugin.md](04-dde-file-manager-preview-plugin.md)

### 3. 核心功能扩展

通过 C++ 插件接口深度扩展文件管理器的核心功能，包括：

- 角标扩展（emblem icon）
- 文件操作拦截和处理
- 窗口事件插件
- 菜单扩展（C++ 接口）
- 详细规范：[dfm-extension-plugin.md](05-dfm-extension-plugin.md)
- 示例代码：[examples/dfm-extension-example](https://github.com/linuxdeepin/dde-file-manager/tree/master/examples/dfm-extension-example)

### 4. 新建文档模板

通过模板文件扩展右键菜单的"新建文档"功能，用户可以快速创建预设格式的文件。

- 详细规范：[new-document-template.md](03-new-document-template.md)

## 快速导航

### 根据需求选择扩展类型

| 需求场景 | 推荐扩展类型 | 文档链接 |
|---------|------------|---------|
| 添加右键菜单项，执行脚本或程序 | Custom Menu Script | [custom-menu-script.md](01-custom-menu-script.md) |
| 为特定文件类型提供预览功能 | 预览插件 | [dde-file-manager-preview-plugin.md](04-dde-file-manager-preview-plugin.md) |
| 深度定制文件管理器行为 | 核心功能扩展 | [dfm-extension-plugin.md](05-dfm-extension-plugin.md) |
| 添加新建文档模板 | 新建文档模板 | [new-document-template.txt](03-new-document-template.txt) |

### 扩展复杂度对比

- **低复杂度**：Custom Menu Script、新建文档模板
- **中等复杂度**：预览插件（基于 Qt 的插件机制）
- **高复杂度**：核心功能扩展（需要 C++ 开发能力）

## 适用对象

本规范面向以下开发者：

- **系统集成商**：为定制化 DDE 桌面环境开发专属功能
- **独立软件开发商**：开发通用或行业特定的文件管理扩展
- **社区贡献者**：为开源社区贡献扩展功能
- **高级用户**：通过脚本扩展个人工作流

## 技术要求

不同扩展类型有不同的技术要求：

- **Custom Menu Script**：熟悉 Shell 脚本或任意编程语言，了解 Desktop Entry 文件格式
- **预览插件**：具备 Qt/C++ 或 Qt/Python 开发能力，了解 D-Bus 通信
- **核心功能扩展**：具备 Qt/C++ 开发能力，了解深度文件管理器架构
- **新建文档模板**：无需编程能力，仅需创建模板文件

## 规范文档结构

每份扩展规范文档包含以下章节：

1. **概述**：扩展功能描述和应用场景
2. **前置条件**：开发环境、依赖库、系统要求
3. **实现规范**：接口定义、文件格式、命名约定
4. **开发指南**：开发步骤、调试方法、最佳实践
5. **部署说明**：安装位置、配置方式、激活机制
6. **参考示例**：完整示例代码或配置文件
7. **注意事项**：常见问题、兼容性说明、安全建议

## 版本约定

本规范基于 dde-file-manager 的以下版本编写：

- 主版本：基于最新的 master 分支
- 接口变更：重大变更会在文档中明确标注
- 兼容性：保留旧版接口，标注弃用状态

## 相关资源

- **项目仓库**：https://github.com/linuxdeepin/dde-file-manager
- **DDE 桌面环境**：https://github.com/linuxdeepin
- **XDG 规范**：https://specifications.freedesktop.org

---

本文档遵循 CC BY-NC-SA 4.0 协议，允许在署名、非商业用途、相同方式共享的前提下使用和修改。
