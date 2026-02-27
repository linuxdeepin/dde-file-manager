# DDE 文件管理器 OEM 菜单示例

本目录包含 3 个可直接使用的 OEM 菜单扩展示例，展示了 OEM Menu Script 机制的核心特性。

> **与 Custom Menu Script 的区别**：OEM 菜单使用标准 [Desktop Entry](https://specifications.freedesktop.org/desktop-entry-spec/latest/) 格式（`.desktop` 文件，`[Desktop Entry]` / `[Desktop Action]` 节），安装到 `~/.local/share/deepin/dde-file-manager/oem-menuextensions/` 目录。
> Custom 菜单使用私有 `.conf` 格式（`[Menu Entry]` / `[Menu Action]` 节），安装到 `context-menus/` 目录。

## 示例说明

| 文件名 | 菜单项 | 展示特性 |
|--------|--------|---------|
| `oem-send-to-desktop.desktop` | 发送到桌面 | `X-DFM-NotShowIn` 场景过滤（仅文件管理器显示，桌面不显示）|
| `oem-file-hash.desktop` | 计算哈希值（MD5 / SHA-256）| `Actions` 子菜单 |
| `oem-set-wallpaper.desktop` | 设为桌面壁纸 | `MimeType` + `X-DFM-ExcludeMimeTypes` 过滤，仅对图片显示 |

### oem-send-to-desktop

在文件管理器中右键单个**文件或目录**时显示「发送到桌面」菜单项，在桌面上创建指向该目标的符号链接。

- 仅在**文件管理器**中显示（`X-DFM-NotShowIn=Desktop;` 避免在桌面上循环操作）
- 仅支持**本地文件系统**（`X-DFM-SupportSchemes=file;`）
- 若桌面已存在同名链接，自动加数字后缀避免冲突

### oem-file-hash

右键**任意单个文件**时显示「计算哈希值」父菜单，展开后可选择：
- **计算 MD5** — 使用系统内置 `md5sum`
- **计算 SHA-256** — 使用系统内置 `sha256sum`

计算结果通过桌面通知展示，并自动复制到剪贴板（需要 `xclip` 或 `xsel`）。

### oem-set-wallpaper

右键**图片文件**（`image/*`，排除 SVG、XCF 等矢量/工程格式）时显示「设为桌面壁纸」菜单项。

支持环境（按优先级尝试）：
1. Deepin / UOS（D-Bus `org.deepin.dde.Appearance1.SetMonitorBackground`）
2. 标准 GNOME（`gsettings org.gnome.desktop.background`）
3. XFCE（`xfconf-query`）

## 文件结构

```
oem-menu-example/
├── install.sh              # 安装脚本（安装到用户目录，无需 root）
├── uninstall.sh            # 卸载脚本
├── menus/
│   ├── oem-send-to-desktop.desktop
│   ├── oem-file-hash.desktop
│   └── oem-set-wallpaper.desktop
└── scripts/
    ├── send-to-desktop.sh
    ├── file-hash.sh
    └── set-wallpaper.sh
```

## 安装

```bash
chmod +x install.sh uninstall.sh
./install.sh
```

安装后的目录：

| 类型 | 路径 |
|------|------|
| 菜单配置（`.desktop`） | `~/.local/share/deepin/dde-file-manager/oem-menuextensions/` |
| 辅助脚本 | `~/.local/share/dde-oem-menu-example/scripts/` |

配置文件会被文件管理器**自动热加载**，无需重启。若未立即生效，执行：

```bash
killall dde-file-manager 2>/dev/null; dde-file-manager &
```

## 卸载

```bash
./uninstall.sh
```

## 依赖

| 示例 | 可选依赖 | 说明 |
|------|----------|------|
| oem-file-hash | `xclip` 或 `xsel` | 将哈希值复制到剪贴板（缺失时仅展示通知） |
| oem-set-wallpaper | `gsettings` | 设置壁纸（Deepin/UOS/GNOME 已内置） |

安装可选依赖：

```bash
sudo apt install xclip
```

## 参考

- [OEM Menu Script 规范文档](../../docs/extension/02-oem-menu-script.md)
- [Custom Menu Script 示例](../custom-menu-example/)
- [freedesktop.org Desktop Entry 规范](https://specifications.freedesktop.org/desktop-entry-spec/latest/)
