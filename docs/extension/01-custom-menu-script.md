# DDE 文件管理器自定义右键菜单规范

## 概述

Custom Menu Script 是 DDE 文件管理器提供的一种轻量级右键菜单扩展机制。通过在指定目录下放置符合规范的 `.conf` 配置文件，开发者无需编写任何 C++ 代码，即可向文件管理器和桌面的右键菜单中注入自定义菜单项，并在用户触发时执行指定的命令或脚本。

### 适用场景

- 为特定文件类型（如音频、文档、压缩包）添加快捷处理操作
- 集成第三方工具到文件管理器右键菜单
- 为行业定制系统添加特殊操作入口
- 通过 Shell 脚本扩展个人工作流

### 特性

- 支持最多三级嵌套菜单
- 支持按文件类型（MimeType）、URL 协议（Scheme）、文件后缀过滤显示
- 支持单文件、多文件、单目录、多目录、空白区域等多种触发场景
- 支持按位置插入，控制菜单项在菜单中的顺序
- 支持菜单项名称及命令的动态参数（文件名、路径等）
- 支持区分桌面和文件管理器窗口两种场景
- 配置文件热重载（修改后自动生效，无需重启）
- 支持国际化菜单名称（本地化语言适配）

---

## 前置条件

### 操作系统

- UOS 20 或更高版本
- Deepin V23 或更高版本

### 开发要求

- 熟悉 INI / Desktop Entry 文件格式
- 具备 Shell 脚本或任意编程语言的基础能力

---

## 配置文件规范

### 放置目录

配置文件须放置在以下目录之一方可生效（按优先级顺序）：

| 目录路径 | 说明 |
| :--- | :--- |
| `/usr/etc/deepin/context-menus/` | 系统级，OEM/预装应用场景 |
| `/etc/deepin/context-menus/` | 系统级，管理员级别的全局配置 |
| `$XDG_DATA_HOME/deepin/dde-file-manager/context-menus/` | 用户级，通常为 `~/.local/share/deepin/dde-file-manager/context-menus/` |
| `$XDG_DATA_DIRS/applications/context-menus/` | 系统级，通常为 `/usr/share/applications/context-menus/` |

> **说明：** 当同名文件在多个目录同时存在时，以先被扫描到的目录为准，后续同名文件被忽略。扫描顺序参考 `XDG_DATA_DIRS` 环境变量配置。

### 文件格式

配置文件必须：

- 以 `.conf` 为扩展名
- 使用 **UTF-8** 编码
- 沿用 INI / Desktop Entry 的 `分组` + `Key=Value` 格式
- `=` 两端**不允许**留有空格
- 多值字段使用 `:` 分隔，`:` 两端**不允许**留有空格
- 注释须**另起一行**，以 `#` 开头；字段行尾**不允许**跟随注释

### 文件结构

一个完整的配置文件由以下部分组成：

```
[Menu Entry]            ← 固定入口组，记录全局信息与一级菜单列表
[Menu Action XXXX]      ← 一级或子级菜单组，XXXX 对应 Actions 字段中的标识符
[Menu Action YYYY]      ← 另一个菜单组（可以是子级）
...
```

> `[Menu Entry]` 必须为配置文件第一个有效分组，缺失则整个文件被视为无效文件。

---

## Key 字段参考

### 全局字段（位于 `[Menu Entry]` 组）

| Key | 类型 | 是否必需 | 说明 |
| :--- | :--- | :--- | :--- |
| `Version` | string | **必需** | 配置文件使用的规范版本，当前为 `1.0`。缺失则整个文件无效。 |
| `Sign` | string | 非必需 | 配置文件签名信息（功能预留，当前不做验证）。 |
| `Comment` | string | 非必需 | 配置文件描述，供开发者阅读，不影响有效性。 |
| `Comment[xx_XX]` | localestring | 非必需 | 本地化描述，如 `Comment[zh_CN]`。 |
| `Actions` | string | **必需** | 一级菜单入口标识符列表，多个以 `:` 分隔。至少需要一个有效项，否则文件无效。 |

### 菜单项字段（位于 `[Menu Action XXXX]` 组）

| Key | 类型 | 适用层级 | 是否必需 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| `Name` | string | 所有层级 | **必需（二者存一）** | 菜单项显示名称，支持动态参数（见动态参数章节）。当找不到对应语言的本地化名称时作为兜底显示。 |
| `Name[xx_XX]` | localestring | 所有层级 | **必需（二者存一）** | 本地化菜单项名称，如 `Name[zh_CN]`。优先级：`language_region（如zh_CN）` > `language（如zh）` > `Name`。 |
| `X-DFM-MenuTypes` | string | **仅一级菜单** | **必需** | 支持的文件选中类型，多个以 `:` 分隔（详见选中类型章节）。缺失则该菜单项及其所有子项均作无效处理。 |
| `MimeType` | string | **仅一级菜单** | 非必需 | 支持的文件 MIME 类型白名单，多个以 `:` 分隔。未配置或配置为 `*` 时表示支持所有类型。 |
| `X-DFM-ExcludeMimeTypes` | string | **仅一级菜单** | 非必需 | 排除的 MIME 类型黑名单，多个以 `:` 分隔。未配置时无黑名单。 |
| `X-DFM-SupportSchemes` | string | **仅一级菜单** | 非必需 | 支持的 URL 协议白名单，多个以 `:` 分隔（如 `file:burn:ftp`）。未配置时支持所有协议。 |
| `X-DFM-NotShowIn` | string | **仅一级菜单** | 非必需 | 不在指定场景显示，多个以 `:` 分隔。可选值：`Desktop`（桌面）、`Filemanager`（文件管理器窗口）。配置 `*` 则在所有场景均不显示。未配置时桌面和文件管理器均显示。 |
| `X-DFM-SupportSuffix` | string | **仅一级菜单** | 非必需 | 支持的文件后缀白名单，多个以 `:` 分隔，支持通配符 `*`（如 `7z*`）。未配置或配置为 `*` 时支持所有后缀。 |
| `X-DFM-ParentMenuPath` | string | **仅一级菜单** | 非必需 | 将该菜单项插入到文件管理器预设命名子菜单内（详见 ParentMenuPath 章节）。 |
| `PosNum` | numeric | 所有层级 | 非必需 | 菜单项插入的位置序号（从 1 开始），未配置则以配置文件书写顺序追加到同级菜单末尾。 |
| `PosNum-XXX` | numeric | **仅一级菜单** | 非必需 | 指定选中类型对应的插入位置，`XXX` 为选中类型值（如 `PosNum-SingleFile=1`）。未配置时以 `PosNum` 为默认值。 |
| `Separator` | string | 所有层级 | 非必需 | 分割线位置。可选值：`None`（无）、`Top`（上方）、`Bottom`（下方）、`Both`（上下均有）。未配置默认无分割线。 |
| `Actions` | string | 所有层级 | 与 `Exec` 二选一 | 子菜单入口标识符列表。有此字段则该项为父级菜单，即使同时配置了 `Exec` 也将被忽略。 |
| `Exec` | string | 所有层级 | 与 `Actions` 二选一 | 菜单项执行的命令，仅对最底层（叶子）菜单有效，支持动态参数（见动态参数章节）。父级菜单中该字段被忽略。 |

> **兼容性说明：** 字段名同时支持 `X-DFM-*` 和 `X-DDE-FileManager-*` 两种前缀（如 `X-DFM-MenuTypes` 等同于 `X-DDE-FileManager-MenuTypes`），`PosNum` 等同于 `X-DDE-FileManager-PosNum`，`Separator` 等同于 `X-DDE-FileManager-Separator`。两种写法均可正常解析。

---

## 选中类型（X-DFM-MenuTypes）

`X-DFM-MenuTypes` 指定菜单项在何种文件选中情况下显示，多个值以 `:` 分隔：

| 值 | 说明 |
| :--- | :--- |
| `SingleFile` | 右键单击选中**单个文件** |
| `SingleDir` | 右键单击选中**单个文件夹** |
| `MultiFiles` | 右键选中**多个文件**（纯文件，无文件夹） |
| `MultiDirs` | 右键选中**多个文件夹**（纯文件夹，无文件） |
| `FileAndDir` | 右键选中**文件和文件夹混合**，等同于同时支持 `MultiFiles` 和 `MultiDirs` |
| `BlankSpace` | 右键点击**空白区域**（非任何文件） |

示例：
```ini
# 单个文件和多个文件均可触发
X-DFM-MenuTypes=SingleFile:MultiFiles

# 在空白区域右键时触发
X-DFM-MenuTypes=BlankSpace
```

---

## 动态参数

### 菜单名称动态参数（用于 `Name`）

| 参数 | 说明 |
| :--- | :--- |
| `%d` | 当前目录的文件夹名称（不含路径） |
| `%b` | 焦点文件的文件名（不含路径和扩展名） |
| `%a` | 焦点文件的完整文件名（含扩展名，不含路径） |

> **注意：** Name 中的动态参数从左到右仅第一个有效参数生效，多余的参数无效。

### 命令动态参数（用于 `Exec`）

| 参数 | 说明 |
| :--- | :--- |
| `%p` | 当前目录路径 |
| `%f` | 焦点文件的本地路径（单文件） |
| `%F` | 所有选中文件的本地路径（多文件，空格分隔） |
| `%u` | 焦点文件的 URL 路径（单文件） |
| `%U` | 所有选中文件的 URL 路径（多文件，空格分隔） |

> **注意：** `Exec` 中的动态参数从左到右仅第一个有效参数生效，多余的参数无效。

---

## 菜单层级规则

### 层级约束

- 最多支持**三级**菜单（一级、二级、三级），超过三级的配置项将被忽略。
- 父级菜单通过 `Actions` 字段引用子菜单；叶子菜单通过 `Exec` 字段定义执行命令。
- 若同一菜单组同时包含 `Actions` 和 `Exec`，则 `Actions` 优先，`Exec` 被忽略。
- 若一个菜单项既无 `Actions` 又无有效的 `Exec`，则该项及其所有父级视为无效。

### 数量限制

| 层级 | 最大菜单项数 |
| :--- | :--- |
| 一级菜单 | 50 |
| 二级菜单 | 100 |
| 三级菜单 | 100 |

### 菜单项排序

- 通过 `PosNum` 指定位置（从 1 开始）。
- 若多个菜单项未指定位置（无 `PosNum`），则按更高层级的配置指定顺序追加，若全层级无 `PosNum` 则按 `Actions` 字段书写顺序排列。
- 同名且均包含子菜单的一级菜单项将被**合并**（子菜单项累加到同一父菜单下）。

---

## X-DFM-ParentMenuPath

该字段仅对通过 `[Menu Entry]` 的 `Actions` 定义的**一级菜单**有效，用于将该菜单项插入到文件管理器预设的命名子菜单内。

### 受支持的预设子菜单路径

| 路径名称 | 对应菜单名称 |
| :--- | :--- |
| `open-with` | 打开方式 |
| `new-document` | 新建文档 |
| `display-as` | 显示方式 |
| `sort-by` | 排序方式 |
| `icon-size` | 图标大小 |
| `send-to` | 发送到 |
| `share` | 共享 |

> `PosNum`、`Separator` 在此场景下均有效，用于控制插入位置和分割线。

---

## 注意事项

- `Version` 字段**必须**配置，否则整个配置文件被视为无效文件。
- `X-DFM-MenuTypes` 对于一级菜单为**必需**字段，缺失时该菜单项及其所有子项均被忽略。
- `Actions` 与 `Exec` 不能共存于同一菜单项，`Actions` 优先级更高。
- 最底层（叶子）菜单必须配置 `Exec`，否则视为无效项。
- 配置文件修改后，文件管理器会在**300ms 延迟**后自动热重载，无需手动重启。
- `X-DFM-NotShowIn` 的值大小写不敏感（`Desktop` 与 `desktop` 等价）。
- `Name` 与 `Name[xx_XX]` 至少需要配置一个；建议同时配置 `Name` 作为兜底。
- 本地化名称优先顺序：`Name[language_region]`（如 `Name[zh_CN]`）> `Name[language]`（如 `Name[zh]`）> `Name`。
- 目前 `Icon` 字段**暂未启用**，配置后不会生效。

---

## 完整示例

### 示例一：单个叶子菜单

点击单个文件时，在右键菜单第二位插入"以超级用户打开"：

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

### 示例二：多层级嵌套菜单

一级菜单包含两个子菜单项，子菜单分别以超级用户和普通用户打开：

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

### 示例三：仅在文件管理器中显示、限定文件类型

对 MP3 和 FLAC 音频文件支持"添加到播放队列"操作，并且不在桌面上显示：

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

### 示例四：空白区域菜单 + 动态参数名

在空白区域右键，菜单名称动态显示当前文件夹名称：

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

### 示例五：插入到预设子菜单（发送到）

将"发送到网盘"插入到文件管理器预设的"发送到"子菜单中：

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

---

## 部署说明

### 安装配置文件

```bash
# 系统级安装（需要 root 权限）
sudo cp my-menu.conf /usr/share/applications/context-menus/

# 用户级安装（无需 root）
cp my-menu.conf ~/.local/share/deepin/dde-file-manager/context-menus/
```

### 验证生效

配置文件满足以下条件后即可在下次右键操作时生效（无需重启文件管理器）：

1. 文件放置在正确目录下
2. `Version` 字段存在
3. `[Menu Entry]` 为首个分组
4. `Actions` 字段存在且至少一个引用的菜单组有效
5. 一级菜单具有 `X-DFM-MenuTypes` 字段
6. 叶子菜单具有有效的 `Exec` 字段
