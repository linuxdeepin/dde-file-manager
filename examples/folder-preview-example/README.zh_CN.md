# 文件夹预览插件示例

一个 DDE 文件管理器预览插件，当用户在文件夹上按下**空格键**时，以丰富的可视化界面预览该文件夹的内容。

## 预览界面

```
┌─────────────────────────────────────────────────────────────┐
│  📁  文档                                                   │
│      /home/alice/文档                                       │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌────────────────┐            │
│  │    📄    │  │    📁    │  │      💾        │            │
│  │    12    │  │    3     │  │   4.7 MB       │            │
│  │  个文件  │  │  个子目录│  │   总大小       │            │
│  └──────────┘  └──────────┘  └────────────────┘            │
├─────────────────────────────────────────────────────────────┤
│  名称               类型          大小      修改时间        │
│  ─────────────────────────────────────────────────────────  │
│  📁 归档            文件夹        —        2024-11-01       │
│  📁 报告            文件夹        —        今天 09:35       │
│  📄 budget.xlsx     电子表格      88.2 KB  昨天 18:00       │
│  📄 notes.md        Markdown      4.1 KB   今天 10:22       │
│  📄 photo.jpg       JPEG 图片     2.3 MB   2024-10-15       │
│  …                                                          │
└─────────────────────────────────────────────────────────────┘
```

### 功能特性

- **头部面板** — 文件夹图标、完整名称、绝对路径
- **统计徽章** — 文件数量、子目录数量、递归总大小
- **文件列表** — 可排序的四列：名称 · 类型 · 大小 · 修改时间
- **异步扫描** — 基于 `QtConcurrent`，扫描期间 UI 不卡顿
- **自适应主题** — 运行时自动识别深色 / 浅色模式并切换配色

## 项目结构

```
folder-preview-example/
├── CMakeLists.txt
├── README.md               # 英文说明
├── README.zh_CN.md         # 中文说明（本文件）
├── debian/
│   ├── changelog           # 版本变更记录
│   ├── compat              # Debhelper 兼容级别
│   ├── control             # 包元数据与构建依赖
│   ├── copyright           # 许可证信息
│   └── rules               # 构建规则（cmake）
├── dde-folder-preview-plugin.json   # MIME 类型元数据
├── folderpreviewplugin.h/.cpp       # Qt 插件工厂类
├── folderpreview.h/.cpp             # AbstractBasePreview 实现
└── foldercontentwidget.h/.cpp       # 主 UI 控件
```

## 依赖项

完整的构建时依赖见 `debian/control`。快速开始至少需要：

```bash
sudo apt install dde-file-manager-dev
```

## 构建与安装

### CMake（开发构建）

```bash
# 配置
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr

# 编译
cmake --build build -j$(nproc)

# 安装（需要 root 权限）
sudo cmake --build build --target install
```

### Debian 包（用于发行分发）

项目提供了 `debian/` 目录，可在 Deepin / UOS 系统下构建标准 `.deb` 包：

```bash
# 安装 debian/control 中声明的构建依赖
sudo apt build-dep .

# 构建二进制包（不签名）
dpkg-buildpackage -uc -us -nc
```

打包完成后，`.deb` 会将插件自动安装到正确的系统路径。

安装后，插件 `.so` 及 `.json` 元数据文件位于：

```
/usr/lib/<arch>/dde-file-manager/plugins/previews/
```

下次在文件夹上按下**空格键**即可自动激活，无需重启文件管理器。文件管理器会按需启动独立的预览进程（`dde-file-manager-preview`），该进程每次启动时都会重新扫描插件目录。

## 关键实现说明

| 主题 | 说明 |
|------|------|
| **MIME 键** | `inode/directory` — 匹配所有本地目录 |
| **异步扫描** | `QtConcurrent::run` + `QFutureWatcher` 在后台扫描文件列表，主线程不阻塞 |
| **总大小** | 通过 `QProcess` 并行执行 `du -sb <path>`，递归统计包含所有子目录在内的总大小 |
| **文件图标** | `QFileIconProvider` 提供与系统一致的原生图标 |
| **MIME 描述** | `QMimeDatabase::mimeTypeForFile`（`MatchExtension` 模式）获取类型描述 |
| **深色模式** | 通过调色板亮度启发式算法在运行时选择配色方案 |

## 扩展思路

- **显示隐藏文件** — 在头部面板添加 `QAction`，用于切换扫描时是否包含 `QDir::Hidden` 条目
- **双击导航** — 调用文件管理器的 D-Bus API，在新标签页中打开子目录
- **子目录大小** — 对列表中的子目录行异步执行 `du -sb <subdir>`，异步填充大小列
