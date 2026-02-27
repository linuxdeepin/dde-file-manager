# DDE 文件管理器模板示例

本示例展示如何创建和部署文件模板，包括普通文件模板和 Desktop Entry 模板。

## 目录结构

```
template-example/
├── README.md                    # 本文件
├── install.sh                   # 安装脚本
├── uninstall.sh                 # 卸载脚本
├── templates/                   # 模板文件目录
│   ├── simple/                  # 普通文件模板
│   │   ├── README.md
│   │   ├── TODO.md
│   │   └── shell-script.sh
│   └── desktop-entry/           # Desktop Entry 模板
│       ├── python-script.desktop
│       ├── python-script.py
│       ├── cpp-source.desktop
│       └── cpp-source.cpp
```

## 模板说明

### 普通文件模板

位于 `templates/simple/` 目录，直接复制到 `$XDG_TEMPLATES_DIR`（通常为 `~/Templates`）。

- **README.md** - Markdown 项目文档模板
- **TODO.md** - 待办事项列表模板
- **shell-script.sh** - Shell 脚本模板

### Desktop Entry 模板

位于 `templates/desktop-entry/` 目录，安装到 `$XDG_DATA_HOME/templates`（通常为 `~/.local/share/templates`）。

- **python-script** - Python 脚本模板（带本地化名称和图标）
- **cpp-source** - C++ 源文件模板（带 SPDX 许可证头）

## 安装

```bash
./install.sh
```

安装脚本会：
1. 将普通文件模板复制到 `~/Templates`
2. 将 Desktop Entry 模板复制到 `~/.local/share/templates`
3. 提示重启文件管理器和桌面

## 卸载

```bash
./uninstall.sh
```

## 使用

安装并重启后，在文件管理器或桌面右键菜单中选择"新建文档"，即可看到新增的模板。

## 参考

完整规范文档：[新建文档模板规范](../../docs/extension/03-new-document-template.md)
