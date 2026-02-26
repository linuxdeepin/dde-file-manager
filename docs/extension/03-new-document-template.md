# DDE 文件管理器新建文档模板规范

## 概述

新建文档模板功能允许用户通过右键菜单的"新建文档"子菜单快速创建预设格式的文件。DDE 文件管理器支持两种模板类型：普通文件模板和 Desktop Entry 描述的模板，开发者和用户可以通过简单的文件配置扩展此功能。

### 应用场景

- 为特定文件类型提供预设模板（如 Markdown、LaTeX、配置文件等）
- 企业环境中部署标准化文档模板
- 个人用户自定义常用文件模板
- 软件包提供应用相关的文档模板

### 功能特性

- 支持任意文件类型作为模板
- 支持通过 Desktop Entry 文件自定义模板名称和图标
- 支持系统级和用户级模板目录
- 模板按文件名去重，优先级明确

## 前置条件

### 系统要求

- DDE 文件管理器 5.0 或更高版本
- 支持 XDG 用户目录规范的 Linux 系统

### 技术要求

- 了解 Linux 文件系统基础知识
- 了解 Desktop Entry 文件格式（可选，用于高级配置）

## 实现规范

### 模板搜索路径

DDE 文件管理器按以下优先级顺序搜索模板：

| 优先级 | 路径 | 说明 |
|-------|------|------|
| 1 | `$XDG_TEMPLATES_DIR` | 用户模板目录（通常为 `~/Templates`） |
| 2 | `$XDG_DATA_DIRS/templates` | 系统数据目录下的 templates 子目录 |
| 3 | `$XDG_DATA_HOME/templates` | 用户数据目录下的 templates 子目录 |

**路径解析规则**：

- `$XDG_TEMPLATES_DIR`：通过 `xdg-user-dir TEMPLATES` 获取，默认为 `~/Templates`
- `$XDG_DATA_DIRS`：系统数据目录列表，默认为 `/usr/local/share:/usr/share`
- `$XDG_DATA_HOME`：用户数据目录，默认为 `~/.local/share`

### 模板类型

#### 1. 普通文件模板

直接将文件放置在 `$XDG_TEMPLATES_DIR` 目录中，文件管理器会自动识别。

**命名规则**：
- 菜单显示名称：文件的基本名称（不含扩展名）
- 图标：根据文件 MIME 类型自动选择

**示例**：
```
~/Templates/
├── 空白文档.txt
├── Markdown文档.md
└── Python脚本.py
```

用户在右键菜单中会看到：
- "空白文档"（文本图标）
- "Markdown文档"（Markdown 图标）
- "Python脚本"（Python 图标）

#### 2. Desktop Entry 模板

通过 `.desktop` 文件描述模板，支持自定义显示名称、图标和本地化。

**文件位置**：

- 系统级：`/usr/share/templates/` 或 `/usr/local/share/templates/`
- 用户级：`~/.local/share/templates/`

**Desktop Entry 格式**：

```ini
[Desktop Entry]
Name=显示名称
Name[zh_CN]=中文显示名称
Icon=图标名称
URL=模板文件名
```

**字段说明**：

| 字段 | 必需 | 说明 |
|-----|------|------|
| `Name` | 是 | 菜单中显示的名称 |
| `Name[locale]` | 否 | 本地化名称（如 `Name[zh_CN]`） |
| `Icon` | 否 | 图标主题中的图标名称 |
| `URL` | 是 | 模板文件的相对路径（相对于 .desktop 文件所在目录） |

**示例**：

文件结构：
```
/usr/share/templates/
├── word-document.desktop
└── word-document.docx
```

`word-document.desktop` 内容：
```ini
[Desktop Entry]
Name=Word Document
Name[zh_CN]=Word 文档
Icon=application-vnd.openxmlformats-officedocument.wordprocessingml.document
URL=word-document.docx
```

### 去重规则

当多个路径中存在同名模板时，按以下规则处理：

1. **文件名去重**：相同文件名（含扩展名）的模板只显示一次
2. **优先级**：用户目录 > 系统数据目录 > 用户数据目录
3. **类型混合**：普通文件模板和 Desktop Entry 模板独立去重

**示例**：

如果存在以下文件：
```
~/Templates/文档.txt
/usr/share/templates/文档.txt
```

菜单中只显示 `~/Templates/文档.txt`（用户目录优先）。

### 模板创建行为

用户选择模板后，文件管理器执行以下操作：

1. 读取模板文件内容
2. 在当前目录创建新文件，文件名为模板的基本名称
3. 如果文件名冲突，自动添加数字后缀（如 `文档(1).txt`）
4. 将模板内容复制到新文件
5. 选中新创建的文件，进入重命名状态

## 开发指南

### 创建普通文件模板

**步骤 1：准备模板文件**

创建包含预设内容的文件：

```bash
# 创建用户模板目录（如果不存在）
mkdir -p ~/Templates

# 创建文本模板
cat > ~/Templates/"README.md" << 'EOF'
# 项目标题

## 简介

项目描述

## 安装

安装说明

## 使用

使用说明
EOF
```

**步骤 2：验证模板**

1. 重启文件管理器：`killall dde-file-manager`
2. 打开文件管理器，在任意目录右键
3. 选择"新建文档" → "README"

### 创建 Desktop Entry 模板

**步骤 1：准备模板文件和描述文件**

```bash
# 创建系统模板目录（需要 root 权限）
sudo mkdir -p /usr/share/templates

# 创建模板文件
sudo tee /usr/share/templates/python-script.py > /dev/null << 'EOF'
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def main():
    pass

if __name__ == "__main__":
    main()
EOF

# 创建 Desktop Entry 文件
sudo tee /usr/share/templates/python-script.desktop > /dev/null << 'EOF'
[Desktop Entry]
Name=Python Script
Name[zh_CN]=Python 脚本
Icon=text-x-python
URL=python-script.py
EOF
```

**步骤 2：验证模板**

重启文件管理器后，在"新建文档"菜单中应显示"Python 脚本"。

### 本地化支持

Desktop Entry 模板支持多语言名称：

```ini
[Desktop Entry]
Name=LaTeX Document
Name[zh_CN]=LaTeX 文档
Name[zh_TW]=LaTeX 文件
Name[de]=LaTeX-Dokument
Name[fr]=Document LaTeX
Icon=text-x-tex
URL=document.tex
```

文件管理器会根据系统语言自动选择对应的 `Name[locale]` 字段。

### 图标选择

**使用主题图标**（推荐）：

```ini
Icon=text-x-generic
Icon=application-pdf
Icon=text-x-python
```

**使用绝对路径**：

```ini
Icon=/usr/share/pixmaps/myapp-template.png
```

**常用图标名称**：

| 文件类型 | 图标名称 |
|---------|---------|
| 文本文件 | `text-x-generic` |
| Markdown | `text-markdown` |
| Python | `text-x-python` |
| Shell 脚本 | `text-x-script` |
| PDF | `application-pdf` |
| Word | `application-vnd.openxmlformats-officedocument.wordprocessingml.document` |

## 部署说明

### 用户级部署

适用于个人用户自定义模板：

```bash
# 1. 创建模板目录
mkdir -p ~/Templates

# 2. 复制模板文件
cp my-template.txt ~/Templates/

# 3. 重启文件管理器和桌面
killall dde-file-manager
systemctl --user restart dde-shell-plugin@org.deepin.ds.desktop.service
```

### 系统级部署

适用于软件包或系统管理员部署：

**方式一：直接安装**（磐石下无效）

```bash
# 安装模板文件
sudo install -Dm644 template.desktop /usr/share/templates/template.desktop

# 重启文件管理器和桌面
killall dde-file-manager
systemctl --user restart dde-shell-plugin@org.deepin.ds.desktop.service
```

**方式二：通过 Debian 包**

在 `debian/install` 文件中添加：

```
templates/*.desktop usr/share/templates
```

在 `debian/postinst` 中添加：

```bash
#!/bin/sh
set -e

if [ "$1" = "configure" ]; then
    # 通知文件管理器重新加载模板
    killall -HUP dde-file-manager 2>/dev/null || true
fi

#DEBHELPER#

exit 0
```

### 卸载

**用户级模板**：

```bash
rm ~/Templates/my-template.txt
```

**系统级模板（磐石下无效）**：

```bash
sudo rm /usr/share/templates/my-template.desktop
```

- **注：**建议通过 debian 包的方式安装和卸载

卸载后需重启文件管理器生效。

## 参考示例

### 完整示例项目

完整的模板示例项目位于：[examples/template-example](../../examples/template-example)

该示例包含：
- 普通文件模板（README.md、TODO.md、Shell脚本.sh）
- Desktop Entry 模板（Python 脚本、C++ 源文件）
- 自动化安装/卸载脚本

### 示例 1：Markdown 文档模板

**文件**：`~/Templates/Markdown文档.md`

```markdown
# 标题

## 章节 1

内容

## 章节 2

内容
```

### 示例 2：C++ 源文件模板

**模板文件**：`/usr/share/templates/cpp-source.cpp`

```cpp
// SPDX-FileCopyrightText: 2026 Your Name <your.email@example.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>

int main(int argc, char *argv[])
{
    return 0;
}
```

**Desktop Entry**：`/usr/share/templates/cpp-source.desktop`

```ini
[Desktop Entry]
Name=C++ Source File
Name[zh_CN]=C++ 源文件
Icon=text-x-c++src
URL=cpp-source.cpp
```

### 示例 3：配置文件模板

**模板文件**：`/usr/share/templates/config.ini`

```ini
[General]
# 通用配置

[Advanced]
# 高级配置
```

**Desktop Entry**：`/usr/share/templates/config.desktop`

```ini
[Desktop Entry]
Name=Configuration File
Name[zh_CN]=配置文件
Icon=text-x-generic
URL=config.ini
```

## 注意事项

### 生效时机

- **模板加载时机**：文件管理器和桌面启动时加载模板列表
- **生效方式**：添加或删除模板后，必须重启文件管理器和桌面才能生效
- **重启命令**：
  - `killall dde-file-manager`（关闭文件管理器进程，需要手动启动文件管理器）
  - `systemctl --user restart dde-shell-plugin@org.deepin.ds.desktop.service`（桌面将自动重启）


### 文件权限

- 模板文件必须具有读权限
- 系统级模板通常设置为 `644` 权限
- 用户级模板继承用户默认权限

```bash
# 设置正确的权限
chmod 644 ~/Templates/my-template.txt
sudo chmod 644 /usr/share/templates/my-template.txt
```

### 文件名限制

- 避免使用特殊字符（如 `/`、`\`、`:`、`*`、`?`、`"`、`<`、`>`、`|`）
- 文件名长度建议不超过 255 字节
- 支持 UTF-8 编码的中文文件名

### 性能考虑

- 模板数量建议控制在 50 个以内，过多会影响菜单加载速度
- 模板文件大小建议不超过 1MB，大文件会影响创建速度
- Desktop Entry 文件应尽量精简

### 兼容性

- **XDG 规范兼容**：遵循 Freedesktop.org 的 XDG 用户目录规范
- **跨桌面环境**：模板机制与 GNOME Nautilus、KDE Dolphin 等文件管理器兼容
- **版本要求**：DDE 文件管理器 5.0+ 完全支持，早期版本可能仅支持用户模板目录

### 常见问题

#### 1. 模板不显示

**可能原因**：
- 模板目录路径不正确
- 文件权限不足
- 文件管理器未重启

**解决方法**：
```bash
# 检查模板目录
xdg-user-dir TEMPLATES
ls -la ~/Templates

# 检查系统模板目录
ls -la /usr/share/templates
ls -la ~/.local/share/templates

# 重启文件管理器
killall dde-file-manager
```

#### 2. Desktop Entry 模板无效

**可能原因**：
- `.desktop` 文件格式错误
- `URL` 字段指向的文件不存在
- 文件编码不是 UTF-8

**解决方法**：
```bash
# 验证 Desktop Entry 文件
desktop-file-validate /usr/share/templates/my-template.desktop

# 检查文件编码
file -i /usr/share/templates/my-template.desktop
```

#### 3. 中文名称显示乱码

**原因**：文件编码不是 UTF-8

**解决方法**：
```bash
# 转换文件编码为 UTF-8
iconv -f GBK -t UTF-8 template.desktop -o template.desktop.utf8
mv template.desktop.utf8 template.desktop
```

#### 4. 图标不显示

**可能原因**：
- 图标名称错误
- 图标主题中不存在该图标

**解决方法**：
```bash
# 查找可用图标
find /usr/share/icons -name "*text*" -type f

# 使用通用图标
Icon=text-x-generic
```

### 安全建议

- 不要在模板中包含敏感信息（密码、密钥等）
- 系统级模板应由可信来源提供
- 定期审查用户模板目录，防止恶意文件

### 最佳实践

1. **命名规范**：使用清晰、描述性的文件名
2. **内容精简**：模板应包含必要的结构，避免过多预设内容
3. **注释说明**：在模板中添加注释，指导用户使用
4. **版本控制**：对模板文件进行版本管理
5. **测试验证**：部署前在测试环境验证模板功能

## 附录

### 相关规范

- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html)
- [Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html)
- [Icon Theme Specification](https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html)
