# deepin/DDE 品牌术语规范

> 规范 deepin 和 DDE 品牌名称在文档、代码中的正确使用方式。

---

## 概览

本规范定义 deepin 和 DDE 品牌术语的标准使用方式，确保项目内文档和代码的一致性。

---

## deepin 品牌规范

### 基本规则

**deepin** 品牌名称在任何文档、图片、代码注释中都应使用**全小写**，即使是段落首字母。

```cpp
// ✅ 正确
// deepin 开源项目
// deepin desktop environment

// ❌ 错误
// Deepin 开源项目
// DEEPIN 开源项目
```

### 代码中的使用

在代码中，deepin 也应使用全小写，除非所在语言的编码规范强制要求全大写或首字母大写。

```cpp
// ✅ 正确
#define DEEPIN_MACRO XXXX     // 宏命名遵循编码规范
const int kDeepinNumber = 1;  // 常量命名遵循编码规范

// ✅ 正确 - 版权信息
// Copyright (c) 2021. deepin All rights reserved.
```

### 文件名中的使用

在文件路径和文件名中，deepin 必须使用全小写。

```bash
# ✅ 正确
/usr/lib/deepin-daemon/dde-system-daemon
/usr/share/deepin/msc/res

# ❌ 错误
/usr/share/Deepin/msc/res
/usr/lib/DeepinDaemon
```

---

## DDE 品牌规范

### 基本规则

**DDE** 是 Deepin Desktop Environment 的缩写。

- **文档中**：使用全大写 `DDE`
- **文件名中**：使用全小写 `dde`

```cpp
// ✅ 正确 - 文档中
The DDE is comprised of the Desktop Environment, deepin Window Manager, Control Center, Launcher and Dock.

// ❌ 错误 - 文档中
Use dde in other os.      // 文档中不允许全小写
Login to Dde.              // 文档中不允许混合大小写

// ✅ 正确 - 文件名中
/usr/lib/deepin-daemon/dde-system-daemon
org.deepin.DDE1.Accounts

// ❌ 错误 - 文件名中
/usr/lib/deepin-daemon/DDE-System-Daemon
com.deepin.daemon        // 旧命名风格，已废弃
```

### 专有名词

**Deepin Desktop Environment** 是专有名词，不要拆开使用或修改大小写。

```cpp
// ✅ 正确
Deepin Desktop Environment

// ❌ 错误
deepin Desktop Environment
deepin desktop environment
```

---

## 文档和注释格式

### deepin 风格注释

在文档中，应该使用标准化的注释格式标记重要信息。

```markdown
<!-- ✅ 正确 - 重要信息标记 -->
> **注意**: deepin 桌面环境使用轻量级设计，避免过度装饰。
> **警告**: 此操作会导致数据丢失，请谨慎使用。
> **提示**: 快捷键 Ctrl+S 可以保存当前配置。

<!-- ✅ 正确 - 代码注释 -->
// 注意 deepin 品牌名称必须全小写
// 警告此函数非线程安全
// 提示缓存会自动清理，无需手动删除
```

---

## 项目和文件创建

### 新项目创建

在创建新项目前，应当检查是否可以扩展现有的 deepin 项目。

```bash
# ✅ 正确 - 扩展现有项目
# 不创建新的项目，而是在现有的组件中添加新功能
# 例如：在 dde-dock 中添加新插件，而不是创建 d-dock-extensions

# ✅ 正确 - 创建新项目
# 当功能无法在现有项目中扩展时，创建新项目
deepin-new-feature       # 全小写 + 连字符
```

### 项目命名

deepin 项目应使用**全小写**，单词使用连字符（`-`）连接。

```bash
# ✅ 正确
plymouth-theme-deepin
deepin-font-manager

# ❌ 错误
Roboto-Autotest          # 大写
deepin_font_manager      # 使用下划线
```

### 应用程序

应用程序（桌面文件中）应使用**倒置域名**格式。

```ini
# ✅ 正确 - desktop 文件
[Desktop Entry]
Name=Deepin Music
# 文件: org.deepin.lianliankan.desktop

# ✅ 正确 - DBus 服务名
org.deepin.dde-launcher
org.deepin.dde-file-manager

# ❌ 错误
deepin-music             # 应用程序应该使用倒置域名
com.deepin.daemon        # 旧命名风格，已废弃
```

### 专有名词例外

当 deepin/DDE 与其他名词组成专有名词时，可以使用大小写混合。

```ini
# ✅ 正确 - 专有名词
[Desktop Entry]
Name=Deepin Music          # 这是一个专有名词，不能拆开

# ⚠️ 注意
这是一个专有名词，在任何情况下都不可以拆开使用。
```

---

## DBus 命名规范

详见 [DBus 接口命名规范](./dbus-conventions.md)。

### 快速参考

| 元素 | 格式 | 示例 |
|------|------|------|
| 服务名 | `org.deepin.DDE1.Component` | `org.deepin.DDE1.Accounts` |
| 对象路径 | `/org/deepin/dde/Component1` | `/org/deepin/dde/accounts` |
| 接口 | `org.deepin.DDE1.Component` | `org.deepin.DDE1.Accounts` |
| 方法 | 大小写混合 | `SetBrightness` |
| 属性 | 大小写混合 | `Volume` |
| 信号 | 大小写混合 | `BrightnessChanged` |

---

## 快速参考

| 场景 | deepin 格式 | DDE 格式 |
|------|------------|----------|
| 文档/注释 | 全小写 `deepin` | 全大写 `DDE` |
| 文件名 | 全小写 `deepin` | 全小写 `dde` |
| 宏/常量 | 编码规范优先 | 编码规范优先 |
| 专有名词 | `Deepin Desktop Environment` | `Deepin Desktop Environment` |
| 项目命名 | `deepin-font-manager` | `dde-dock` |

---

## 参考资源

- [DBus API Design Guidelines](https://dbus.freedesktop.org/doc/dbus-api-design.html)
- [Filesystem Hierarchy Standard (FHS)](https://refspecs.linuxfoundation.org/FHS_3.0/fhs-3.0.html)
