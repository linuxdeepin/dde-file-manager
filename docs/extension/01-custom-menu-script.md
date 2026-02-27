# DDE 文件管理器自定义右键菜单规范

## 概述

Custom Menu Script 是 DDE 文件管理器提供的一种**轻量级右键菜单扩展机制**。通过在指定目录下放置符合规范的 `.conf` 配置文件，开发者无需编写任何 C++ 代码，即可向文件管理器和桌面的右键菜单中注入自定义菜单项，并在用户触发时执行指定的命令或脚本。

### 适用场景

- 为特定文件类型（如音频、文档、压缩包）添加快捷处理操作
- 将第三方工具集成到文件管理器右键菜单
- 为行业定制系统添加特殊操作入口
- 通过 Shell 脚本或可执行程序扩展个人工作流

### 主要特性

| 特性 | 说明 |
| :--- | :--- |
| 零代码扩展 | 仅需配置文件和脚本，无需 C++ 开发 |
| 三级嵌套菜单 | 最多支持三级子菜单结构 |
| 精细过滤控制 | 按 MimeType、URL 协议、文件后缀过滤显示 |
| 多场景触发 | 支持单文件、多文件、目录、空白区域等 |
| 热重载 | 配置文件修改后约 300ms 内自动生效 |
| 国际化支持 | 支持本地化菜单名称（`Name[zh_CN]` 等） |
| 位置控制 | 通过 `PosNum` 精确控制菜单项插入位置 |
| 场景隔离 | 可区分桌面和文件管理器窗口两种场景 |

---

## 前置条件

### 操作系统要求

- UOS 20 或更高版本
- Deepin V23 或更高版本

### 运行时依赖

Custom Menu Script 机制由文件管理器内置的 `dfmplugin-menu` 插件提供，无需额外安装依赖。

> 配置文件中 `Exec` 所调用的外部命令（如 Shell 脚本、第三方工具），需由开发者自行确保已安装在目标系统上。

### 开发要求

- 熟悉 INI / Desktop Entry 格式（`[Section]` + `Key=Value` 的键值对格式）
- 具备 Shell 脚本或任意编程语言的基础能力
- 了解 MIME 类型（可通过 `file --mime-type <file>` 或 `xdg-mime query filetype <file>` 查询）

---

## 实现规范

### 配置文件格式

配置文件必须满足以下要求：

- **扩展名**：以 `.conf` 结尾
- **编码**：UTF-8
- **格式**：INI / Desktop Entry 的 `[分组]` + `Key=Value` 结构
- **等号**：`=` 两端**不允许**留有空格
- **多值分隔**：多个值以 `:` 分隔，`:` 两端**不允许**留有空格
- **注释**：须**另起一行**，以 `#` 开头；字段行尾**不允许**追加注释

### 文件结构

```
[Menu Entry]            ← 固定入口组（必须为文件的第一个有效分组）
[Menu Action XXXX]      ← 一级或子级菜单组，XXXX 为 Actions 中的标识符
[Menu Action YYYY]      ← 另一个菜单组（可为子级）
...
```

> `[Menu Entry]` 缺失，或不是文件中第一个有效分组时，整个文件被视为**无效文件**并跳过。

### 全局字段（`[Menu Entry]` 组）

| Key | 类型 | 是否必需 | 说明 |
| :--- | :--- | :--- | :--- |
| `Version` | string | **必需** | 规范版本，当前为 `1.0`。缺失则整个文件无效。 |
| `Actions` | string | **必需** | 一级菜单入口标识符列表，多个以 `:` 分隔。至少需要一个有效项。 |
| `Comment` | string | 非必需 | 配置文件描述，供开发者阅读。 |
| `Comment[xx_XX]` | localestring | 非必需 | 本地化描述，如 `Comment[zh_CN]`。 |
| `Sign` | string | 非必需 | 签名预留字段，当前不做验证。 |

### 菜单项字段（`[Menu Action XXXX]` 组）

| Key | 别名 | 类型 | 适用层级 | 是否必需 | 说明 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `Name` | — | string | 所有层级 | **二者选一** | 菜单项显示名称，支持动态参数。找不到本地化名称时作为兜底。 |
| `Name[xx_XX]` | — | localestring | 所有层级 | **二者选一** | 本地化名称，如 `Name[zh_CN]`。优先级：`language_region` > `language` > `Name`。 |
| `X-DFM-MenuTypes` | `X-DDE-FileManager-MenuTypes` | string | **仅一级菜单** | **必需** | 触发场景类型，多个以 `:` 分隔（见 [选中类型](#选中类型-x-dfm-menutypes)）。缺失则该项及所有子项均无效。 |
| `MimeType` | — | string | **仅一级菜单** | 非必需 | MIME 类型白名单，多个以 `:` 分隔。未配置或为 `*` 时匹配所有类型。 |
| `X-DFM-ExcludeMimeTypes` | `X-DDE-FileManager-ExcludeMimeTypes` | string | **仅一级菜单** | 非必需 | MIME 类型黑名单，多个以 `:` 分隔。 |
| `X-DFM-SupportSchemes` | `X-DDE-FileManager-SupportSchemes` | string | **仅一级菜单** | 非必需 | URL 协议白名单，如 `file:burn:ftp`。未配置时支持所有协议。 |
| `X-DFM-NotShowIn` | `X-DDE-FileManager-NotShowIn` | string | **仅一级菜单** | 非必需 | 不显示的场景：`Desktop`（桌面）、`Filemanager`（文件管理器窗口）、`*`（全部不显示）。值不区分大小写。 |
| `X-DFM-SupportSuffix` | `X-DDE-FileManager-SupportSuffix` | string | **仅一级菜单** | 非必需 | 文件后缀白名单，支持通配符 `*`（如 `7z*`）。未配置或为 `*` 时支持所有后缀。 |
| `X-DFM-ParentMenuPath` | — | string | **仅一级菜单** | 非必需 | 插入到文件管理器预设命名子菜单内（见 [X-DFM-ParentMenuPath](#x-dfm-parentmenupath)）。 |
| `PosNum` | `X-DDE-FileManager-PosNum` | numeric | 所有层级 | 非必需 | 插入位置序号（从 1 开始）。未配置则追加到同级末尾。 |
| `PosNum-XXX` | — | numeric | **仅一级菜单** | 非必需 | 指定选中类型对应的位置，`XXX` 为选中类型值（如 `PosNum-SingleFile=1`）。 |
| `Separator` | `X-DDE-FileManager-Separator` | string | 所有层级 | 非必需 | 分割线：`None`（无）、`Top`（上方）、`Bottom`（下方）、`Both`（上下）。默认无。 |
| `Actions` | — | string | 所有层级 | 与 `Exec` 二选一 | 子菜单标识符列表。存在此字段则为父级菜单，同时存在的 `Exec` 被忽略。 |
| `Exec` | — | string | 所有层级 | 与 `Actions` 二选一 | 叶子菜单执行的命令，支持动态参数（见 [动态参数](#动态参数)）。父级菜单中此字段被忽略。 |

### 选中类型（X-DFM-MenuTypes）

| 值 | 说明 |
| :--- | :--- |
| `SingleFile` | 右键单击选中**单个文件** |
| `SingleDir` | 右键单击选中**单个文件夹** |
| `MultiFiles` | 右键选中**多个文件**（纯文件，无文件夹） |
| `MultiDirs` | 右键选中**多个文件夹**（纯文件夹，无文件） |
| `FileAndDir` | 右键选中**文件和文件夹混合**（等同于同时配置 `MultiFiles:MultiDirs`） |
| `BlankSpace` | 右键点击**空白区域** |

```ini
# 单文件和多文件均可触发
X-DFM-MenuTypes=SingleFile:MultiFiles

# 空白区域右键时触发
X-DFM-MenuTypes=BlankSpace
```

### 动态参数

#### 菜单名称动态参数（用于 `Name`）

| 参数 | 说明 |
| :--- | :--- |
| `%d` | 当前目录的文件夹名称（不含路径） |
| `%b` | 焦点文件的文件名（不含路径和扩展名） |
| `%a` | 焦点文件的完整文件名（含扩展名，不含路径） |

#### 命令动态参数（用于 `Exec`）

| 参数 | 说明 |
| :--- | :--- |
| `%p` | 当前目录路径 |
| `%f` | 焦点文件的本地路径（单文件场景） |
| `%F` | 所有选中文件的本地路径（空格分隔，多文件场景） |
| `%u` | 焦点文件的 URL（单文件场景） |
| `%U` | 所有选中文件的 URL（空格分隔，多文件场景） |

> **注意：** `Name` 和 `Exec` 中的动态参数均为**从左到右仅第一个有效参数生效**，多余参数被忽略。

### 菜单层级规则

#### 层级约束

- 最多支持**三级**菜单（一、二、三级），超过三级的配置项被忽略。
- 父级菜单通过 `Actions` 引用子菜单组；叶子菜单通过 `Exec` 定义执行命令。
- 同一菜单组中 `Actions` 与 `Exec` 不可共存，`Actions` 优先，`Exec` 被忽略。
- 既无 `Actions` 又无有效 `Exec` 的菜单项视为无效，其所有父级也随之无效。

#### 数量限制

| 层级 | 最大菜单项数 |
| :--- | :--- |
| 一级菜单 | 50 |
| 二级菜单 | 100 |
| 三级菜单 | 100 |

#### 菜单项排序

- 通过 `PosNum` 指定位置（从 1 开始）。未配置时，按 `Actions` 书写顺序追加到同级末尾。
- 同名且均含子菜单的一级菜单项将自动**合并**（子菜单项累加到同一父菜单下）。
- 同一目录内，文件按**修改时间倒序**加载；跨目录时以**先扫描到的目录**为准（后续重复文件名跳过）。

### 命名约定

- `.conf` 文件名建议使用全小写加连字符，如 `compress-here.conf`，避免与其他工具配置冲突。
- `[Menu Action XXXX]` 中的标识符（`XXXX`）建议使用 `PascalCase`，在同一配置文件内保持唯一。
- 本地化语言后缀遵循标准 locale 格式：优先使用 `Name[zh_CN]`，其次 `Name[zh]`，最后兜底 `Name`。

### X-DFM-ParentMenuPath

将一级菜单项插入到文件管理器预设的命名子菜单中。仅对 `[Menu Entry]` 中 `Actions` 定义的**一级菜单**有效。

| 路径名称 | 对应菜单名称 |
| :--- | :--- |
| `open-with` | 打开方式 |
| `new-document` | 新建文档 |
| `display-as` | 显示方式 |
| `sort-by` | 排序方式 |
| `icon-size` | 图标大小 |
| `send-to` | 发送到 |
| `share` | 共享 |

> `PosNum`、`Separator` 在此场景下均有效。

---

## 开发指南

### 开发步骤

**第一步：确认触发场景**

根据需求确定菜单应在何种情况下出现：右键某类文件、目录，还是空白区域。参考 [选中类型](#选中类型-x-dfm-menutypes) 选择 `X-DFM-MenuTypes` 值。

**第二步：设计菜单结构**

- 单操作 → 一级叶子菜单（`Exec` 直接执行命令）
- 多操作 → 二级子菜单（一级使用 `Actions`，二级各自 `Exec`）

**第三步：编写配置文件**

从以下最小骨架出发，逐步添加过滤条件和本地化名称：

```ini
[Menu Entry]
Version=1.0

Actions=MyAction

[Menu Action MyAction]
Name=My Action
Name[zh_CN]=我的操作
X-DFM-MenuTypes=SingleFile
Exec=/path/to/my-script.sh %f
```

**第四步：编写执行脚本**

- 将执行逻辑封装为独立脚本（`.sh`、Python、任意可执行文件均可）
- 赋予可执行权限：`chmod +x my-script.sh`
- 在 `Exec` 中使用**绝对路径**引用脚本，避免依赖 `PATH` 环境变量

**第五步：安装并验证（用户级，无需 root）**

```bash
MENU_DIR="$HOME/.local/share/deepin/dde-file-manager/context-menus"
mkdir -p "$MENU_DIR"
cp my-menu.conf "$MENU_DIR/"
# 约 300ms 后右键菜单自动更新
```

### 调试方法

**从终端启动文件管理器，观察解析日志：**

```bash
dde-file-manager 2>&1 | grep -i "custom\|loading\|menu"
# 或通过 systemd 用户日志
journalctl --user -f | grep dde-file-manager
```

**常见加载失败原因排查：**

| 现象 | 排查方向 |
| :--- | :--- |
| 菜单完全不出现 | 检查 `Version` 是否存在；`[Menu Entry]` 是否为首个分组；`X-DFM-MenuTypes` 是否配置 |
| 菜单出现但点击无响应 | 检查 `Exec` 脚本是否有可执行权限（`chmod +x`），路径是否正确 |
| 只在部分情况出现 | 检查 `MimeType`、`X-DFM-SupportSchemes`、`X-DFM-NotShowIn` 的过滤条件 |
| 热重载不生效 | 确认目录在文件管理器**启动时**已存在（新建目录须重启文件管理器） |

**在终端直接验证脚本：**

```bash
# 将 Exec 命令直接在终端执行，排除文件管理器因素
/path/to/my-script.sh /home/user/test-file.txt
```

**查询文件的 MIME 类型：**

```bash
file --mime-type /path/to/file
# 或
xdg-mime query filetype /path/to/file
```

### 最佳实践

- **脚本使用绝对路径**：`Exec` 字段中务必使用脚本的绝对路径，文件管理器执行时的 `PATH` 可能与终端不同。
- **用户反馈**：脚本执行结果不可见，建议通过 `notify-send` 给用户反馈操作结果（成功或失败）。
- **优雅的依赖检查**：脚本开头检测依赖工具是否存在，不存在时通过通知提示用户安装，而非静默失败。
- **`%F` 与多文件**：`X-DFM-MenuTypes=MultiFiles` 时，`Exec` 使用 `%F` 接收所有选中文件路径（空格分隔）。若路径含空格，需在脚本内用引号包裹变量。
- **`Name` 兜底**：始终同时配置不含语言后缀的 `Name` 作为兜底，避免非中文环境下显示为空。
- **多值字段无空格**：`X-DFM-MenuTypes=SingleFile:MultiFiles` 中冒号两侧不能有空格，否则解析失败。

---

## 部署说明

### 配置文件放置目录

文件管理器按以下优先级顺序扫描目录，**同名文件以先扫描到的目录为准**：

| 优先级 | 目录路径 | 说明 |
| :---: | :--- | :--- |
| 1（最高） | `/usr/etc/deepin/context-menus/` | 系统级（OEM/预装场景） |
| 2 | `/etc/deepin/context-menus/` | 系统级（管理员全局配置，需 root） |
| 3 | `~/.local/share/deepin/dde-file-manager/context-menus/` | 用户级（推荐开发调试使用，无需 root） |
| 4（最低） | `$XDG_DATA_DIRS/applications/context-menus/` | 系统数据目录（通常为 `/usr/share/applications/context-menus/`） |

> `$XDG_DATA_HOME` 默认为 `~/.local/share`；`$XDG_DATA_DIRS` 中配置的各路径均会扫描，但仅在目录**已存在**时生效。

### 安装命令

```bash
# 用户级安装（推荐，无需 root）
MENU_DIR="$HOME/.local/share/deepin/dde-file-manager/context-menus"
mkdir -p "$MENU_DIR"
cp my-menu.conf "$MENU_DIR/"

# 系统级安装（需要 root）
sudo mkdir -p /usr/share/applications/context-menus/
sudo cp my-menu.conf /usr/share/applications/context-menus/
```

### 热重载机制

文件管理器在**启动时**对所有已存在的配置目录注册文件监听器（基于 `AbstractFileWatcher`）。当目录内文件发生变化时，触发 300ms 延迟防抖后重新加载，无需手动重启。

```
文件变化事件
    └─► fileAttributeChanged 信号
            └─► QTimer::start(300ms) 防抖
                    └─► 重新扫描所有配置目录
                            └─► 右键菜单下次触发时生效
```

> **注意：** 热重载仅对文件管理器**启动时已存在**的目录有效。若新建了配置目录，需重启文件管理器才能开始监听。

### 验证生效条件

配置文件须同时满足以下条件才会显示菜单项：

1. 文件放置在正确目录下，且该目录在文件管理器启动时已存在
2. `[Menu Entry]` 为文件中**第一个**有效分组
3. `Version` 字段存在且非空
4. `Actions` 字段存在且至少一个引用的菜单组有效
5. 一级菜单组配置了 `X-DFM-MenuTypes` 字段
6. 叶子菜单组配置了有效的 `Exec` 字段

---

## 参考示例

### 示例：单个叶子菜单

右键单个文件时，在菜单第二位插入"以超级用户打开"：

```ini
[Menu Entry]
Version=1.0
Comment=Open file with superuser
Comment[zh_CN]=以超级用户打开示例

Actions=OpenAsSuperUser

[Menu Action OpenAsSuperUser]
Name=Open as Superuser
Name[zh_CN]=以超级用户打开
X-DFM-MenuTypes=SingleFile
PosNum=2
Exec=pkexec xdg-open %f
```

### 示例：二级嵌套菜单

一级菜单包含两个子项，分别以超级用户和普通用户打开：

```ini
[Menu Entry]
Version=1.0
Comment=Open with user level selection
Comment[zh_CN]=选择用户级别打开

Actions=OpenWithLevel

[Menu Action OpenWithLevel]
Name=Open As...
Name[zh_CN]=用户级别打开...
X-DFM-MenuTypes=SingleFile:MultiFiles
PosNum=3
Separator=Top
Actions=OpenAsSuperUser:OpenAsNormalUser

[Menu Action OpenAsSuperUser]
Name=Open as Superuser
Name[zh_CN]=以超级用户打开
PosNum=1
Exec=pkexec xdg-open %f

[Menu Action OpenAsNormalUser]
Name=Open as Normal User
Name[zh_CN]=以普通用户打开
PosNum=2
Exec=xdg-open %f
```

### 示例：限定文件类型 + 仅文件管理器显示

对 MP3 和 FLAC 音频文件支持"添加到播放队列"，不在桌面显示：

```ini
[Menu Entry]
Version=1.0
Comment=Audio player plugin menus

Actions=AddToQueue

[Menu Action AddToQueue]
Name=Add to Play Queue
Name[zh_CN]=添加到播放队列
X-DFM-MenuTypes=SingleFile:MultiFiles
MimeType=audio/mpeg:audio/flac
X-DFM-SupportSchemes=file
X-DFM-NotShowIn=Desktop
PosNum=1
Exec=/usr/bin/my-player --add-queue %F
```

### 示例：空白区域菜单 + 动态名称参数

空白区域右键，菜单名称显示当前文件夹名称：

```ini
[Menu Entry]
Version=1.0
Comment=Run script in current directory

Actions=RunHereScript

[Menu Action RunHereScript]
Name=Run Script Here (%d)
Name[zh_CN]=在此处(%d)运行脚本
X-DFM-MenuTypes=BlankSpace
PosNum=5
Exec=/usr/bin/my-script --dir %p
```

### 示例：插入到预设子菜单（发送到）

将"发送到网盘"插入到文件管理器预设的"发送到"子菜单：

```ini
[Menu Entry]
Version=1.0
Comment=Send to cloud drive

Actions=SendToCloud

[Menu Action SendToCloud]
Name=Cloud Drive
Name[zh_CN]=网盘
X-DFM-MenuTypes=SingleFile:MultiFiles:SingleDir:MultiDirs
X-DFM-ParentMenuPath=send-to
PosNum=1
Exec=/usr/bin/cloud-uploader %F
```

### 可运行完整示例（custom-menu-example）

项目提供了三个真实可运行的完整示例，含配置文件、辅助脚本及安装/卸载脚本：

> 路径：[`examples/custom-menu-example/`](../../examples/custom-menu-example/)

| 示例配置文件 | 触发条件 | 菜单结构 | 说明 |
| :--- | :--- | :--- | :--- |
| `compress-here.conf` | 右键文件/目录（单/多） | 二级子菜单 | 将所选文件压缩为 `.tar.gz` 或 `.zip`，输出到同目录 |
| `copy-path.conf` | 右键单个文件/目录 | 单层菜单 | 将所选项的绝对路径写入系统剪贴板（需 `xclip` 或 `xsel`） |
| `open-terminal-here.conf` | 右键空白区域或单个目录 | 单层菜单 | 在对应目录启动终端模拟器（优先使用 `deepin-terminal`） |

**安装方式（用户级，无需 root）：**

```bash
cd examples/custom-menu-example
chmod +x install.sh uninstall.sh
./install.sh
```

安装脚本会将辅助脚本部署到 `~/.local/share/dde-custom-menu-example/scripts/`，配置文件（已自动替换脚本路径）安装到 `~/.local/share/deepin/dde-file-manager/context-menus/`，约 300ms 后菜单自动生效。卸载执行 `./uninstall.sh`。

---

## 注意事项

### 常见问题

| 问题 | 原因 | 解决方案 |
| :--- | :--- | :--- |
| 菜单完全不显示 | `Version` 或 `[Menu Entry]` 缺失，或 `[Menu Entry]` 不是第一个分组 | 检查文件结构，确保 `[Menu Entry]` 在所有 `[Menu Action ...]` 之前 |
| 菜单显示但点击无效 | `Exec` 脚本无可执行权限，或路径错误 | `chmod +x` 并使用绝对路径 |
| 某类文件不显示菜单 | `MimeType` 或 `X-DFM-SupportSuffix` 过滤 | 用 `file --mime-type` 确认文件类型后更新配置 |
| 修改后不生效 | 配置目录在启动后新建，未被监听 | 重启文件管理器使其重新扫描目录 |
| 多文件时参数缺失 | `Exec` 中使用了 `%f`（单文件参数） | 多文件场景应使用 `%F` |
| 桌面和文件管理器均不显示 | `X-DFM-NotShowIn=*` 或同时配置了 `Desktop:Filemanager` | 去掉该配置或按需指定 `Desktop` 或 `Filemanager` |

### 安全建议

- **避免滥用特权命令**：非必要不在 `Exec` 中使用 `sudo` 或 `pkexec`，评估提权操作在多用户系统中的安全影响。
- **脚本输入校验**：`%f`、`%F` 参数来自文件系统，脚本应对路径进行基本校验，防止构造特殊名称的文件触发非预期行为。
- **系统级配置权限**：`/etc/deepin/context-menus/` 和 `/usr/share/applications/context-menus/` 需 root 权限写入，部署前应评估影响范围。

