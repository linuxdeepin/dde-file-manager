# DDE 文件管理器自定义菜单示例

本示例展示如何通过 Custom Menu Script 机制为 DDE 文件管理器右键菜单添加自定义操作，涵盖单层菜单、多级子菜单及多场景触发等常见用法。

## 目录结构

```
custom-menu-example/
├── README.md                      # 本文件
├── install.sh                     # 安装脚本
├── uninstall.sh                   # 卸载脚本
├── menus/                         # 菜单配置文件
│   ├── compress-here.conf         # 示例一：压缩到...（含二级子菜单）
│   ├── copy-path.conf             # 示例二：复制文件路径
│   └── open-terminal-here.conf   # 示例三：在此处打开终端
└── scripts/                       # 菜单触发时执行的辅助脚本
    ├── compress-here.sh
    ├── copy-path.sh
    └── open-terminal-here.sh
```

## 示例说明

### 示例一：压缩到...（compress-here.conf）

**触发方式：** 右键单个或多个文件/目录

**菜单结构（二级子菜单）：**
```
压缩到...
  ├── 压缩为 .tar.gz
  └── 压缩为 .zip
```

**效果：** 在所选文件的同级目录下生成 `archive_<时间戳>.tar.gz` 或 `archive_<时间戳>.zip`，并通过系统通知提示结果。

**依赖：**
- `.tar.gz`：使用系统内置的 `tar`，无需额外安装。
- `.zip`：需要安装 `zip`（`sudo apt install zip`）；若未安装，将弹出提示。

---

### 示例二：复制路径（copy-path.conf）

**触发方式：** 右键单个文件或目录

**效果：** 将所选项的完整绝对路径（如 `/home/user/Documents/report.pdf`）写入系统剪贴板，并通过通知显示已复制的路径。

**依赖：** `xclip`（推荐）或 `xsel`，二选一：
```bash
sudo apt install xclip
# 或
sudo apt install xsel
```

---

### 示例三：在此处打开终端（open-terminal-here.conf）

**触发方式：**
- 右键**空白区域** → 在当前目录打开终端
- 右键**单个目录** → 在该目录中打开终端

**效果：** 启动终端模拟器并切换到对应目录。按优先级依次尝试以下终端：
`deepin-terminal` → `xfce4-terminal` → `gnome-terminal` → `konsole` → `xterm`

**依赖：** 上述任意一个终端模拟器。Deepin/UOS 系统默认预装 `deepin-terminal`，无需额外安装。

---

## 安装

```bash
chmod +x install.sh
./install.sh
```

安装脚本会：

1. 将辅助脚本复制到 `~/.local/share/dde-custom-menu-example/scripts/`
2. 将菜单配置文件（已替换脚本路径）复制到 `~/.local/share/deepin/dde-file-manager/context-menus/`

> 文件管理器会在约 300ms 内自动重载配置，**无需重启**。

## 卸载

```bash
./uninstall.sh
```

## 自定义与扩展

你可以修改 `menus/` 下的 `.conf` 文件来调整菜单名称、触发条件、插入位置等，修改 `scripts/` 下的脚本来实现不同的操作逻辑。

修改后重新运行 `./install.sh` 即可更新已安装的版本。

## 参考

- 完整规范文档：[自定义右键菜单规范](../../docs/extension/01-custom-menu-script.md)
- 项目仓库：https://github.com/linuxdeepin/dde-file-manager
