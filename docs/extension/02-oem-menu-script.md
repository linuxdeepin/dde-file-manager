# DDE 文件管理器 OEM 菜单扩展规范

## 概述

OEM Menu Script 是 DDE 文件管理器早期提供的一种菜单扩展机制，通过 Desktop Entry 配置文件向文件管理器的右键菜单添加自定义菜单项和执行脚本。

**状态：已弃用** - 此机制仍然兼容，但是新项目建议使用 [Custom Menu Script](01-custom-menu-script.md) 或 [dfm-extension Plugin](05-dfm-extension-plugin.md)。

### 扩展能力

OEM菜单支持以下特性：

| 特性 | 功能描述 |
|------|---------|
| 空白区域菜单 | 在文件管理器/桌面空白区域右键菜单中添加菜单项 |
| 文件选中菜单 | 在选中文件右键菜单中添加菜单项 |
| 子菜单支持 | 支持多级子菜单嵌套 |
| 文件类型过滤 | 根据 MIME 类型、文件后缀过滤显示菜单 |
| URL 方案过滤 | 根据 URL 方案（file、ftp、smb 等）过滤显示 |
| 场景过滤 | 区分桌面和文件管理器两种场景 |
| 命令参数替换 | 自动替换命令中的目录、文件路径参数 |

### 设计特点

1. **配置驱动**：无需编写代码，通过 Desktop Entry 文件即可配置
2. **热更新**：自动监控配置文件变化，实时生效
3. **低侵入性**：不影响文件管理器核心功能
4. **多规则过滤**：支持多种条件组合过滤菜单显示

## 前置条件

### 运行环境

#### 操作系统要求
- UOS 20 或更高版本
- Deepin V20 或更高版本

### 技术要求

- **配置语言**：Desktop Entry 格式（freedesktop.org 规范）
- **脚本语言**：任意可执行程序（Shell、Python、二进制程序等）
- **字符编码**：UTF-8

## Desktop Entry 规范

### 文件结构

OEM 菜单配置文件采用标准 Desktop Entry 格式，由主配置组和子配置组构成：

```ini
[Desktop Entry]
# 主菜单配置

[Desktop Action ActionName1]
# 子菜单1配置

[Desktop Action ActionName2]
# 子菜单2配置
```

### 配置路径

配置文件从以下目录按顺序加载（后加载的同名配置会追加到菜单列表中）：

| 加载顺序 | 路径                                                           | 加载方式 | 说明            |
| ---- | ------------------------------------------------------------ | ---- | ------------- |
| 1    | `/usr/etc/deepin/menu-extensions/`                           | 直接加载 | 默认系统配置目录 |
| 2    | `/etc/deepin/menu-extensions/`                               | 直接加载 | 系统级配置目录 |
| 3    | `/usr/share/deepin/dde-file-manager/oem-menuextensions/`     | **不直接加载** | 系统共享目录，通过自动同步机制分发到用户目录 |
| 4    | `~/.local/share/deepin/dde-file-manager/oem-menuextensions/` | 直接加载 | 用户级配置目录 |

**关键说明**：

1. **加载机制差异**：
   - 前两个目录（`/usr/etc` 和 `/etc`）中的配置文件会被文件管理器**直接加载**
   - `/usr/share` 目录中的配置**不会直接加载**，而是通过自动同步机制复制到用户目录后才生效
   - 用户目录中的配置会被直接加载

2. **菜单项累加**：文件管理器按上述顺序依次加载配置文件，后加载的配置中的菜单项会**追加**到菜单列表中

3. **同名文件处理**：相同文件名的配置不会互相覆盖，而是共存并按加载顺序显示菜单项

4. **用户自定义建议**：如果需要自定义或修改系统菜单，应将配置复制到用户级目录后再编辑


**文件命名规范**：

- 文件扩展名必须为 `.desktop`
- 文件名建议格式：`<vendor>-<feature>.desktop`
- 示例：`mycompany-backup-tool.desktop`

## 字段说明

### Desktop Entry 组字段

#### 基本信息字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `Type` | 字符串 | **是** | 条目类型，固定为 `Application`（Desktop Entry 规范要求） |
| `Name` | 字符串 | **是** | 菜单项显示名称，支持本地化（`Name[zh_CN]`） |
| `Icon` | 字符串 | 否 | 图标名称（主题图标）或绝对路径 |
| `Comment` | 字符串 | 否 | 菜单项说明信息 |
| `Exec` | 字符串 | 条件必需 | 执行的命令，支持参数占位符。**无子菜单时必需**；有子菜单（`Actions`）时可为空，此时点击主菜单项不执行操作，仅展开子菜单 |

#### 菜单类型字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `X-DDE-FileManager-MenuTypes` | 字符串列表 | **是** | 菜单显示场景类型，分号分隔 |
| `X-DFM-MenuTypes` | 字符串列表 | **是** | 同上，别名字段 |

**MenuTypes 可选值**：

| 值 | 说明 | 适用场景 |
|----|------|---------|
| `EmptyArea` | 空白区域菜单 | 在文件管理器/桌面空白处右键 |
| `SingleFile` | 单个文件菜单 | 选中单个文件右键 |
| `SingleDir` | 单个目录菜单 | 选中单个目录右键 |
| `MultiFileDirs` | 多选菜单 | 选中多个文件或目录右键 |

**示例**：
```ini
X-DFM-MenuTypes=SingleFile;SingleDir;MultiFileDirs
```

#### 文件类型过滤字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `MimeType` | 字符串列表 | 否 | 支持的 MIME 类型，分号分隔，支持通配符 |
| `X-DDE-FileManager-ExcludeMimeTypes` | 字符串列表 | 否 | 排除的 MIME 类型 |
| `X-DFM-ExcludeMimeTypes` | 字符串列表 | 否 | 同上，别名字段 |

**MIME 类型匹配规则**：
- **精确匹配**：`text/plain` - 严格匹配指定的 MIME 类型
- **通配符匹配**：`image/*` - 匹配所有图片类型（如 image/png, image/jpeg 等）
- **父类型自动匹配**：文件管理器会自动获取文件 MIME 类型的所有父类型并加入匹配列表
  - 例如：`application/vnd.openxmlformats-officedocument.spreadsheetml.sheet` (xlsx文件) 的父类型包括 `application/zip`
  - 在 `MimeType` 字段匹配时，会检查文件的完整 MIME 类型列表（包括父类型）
  - 在 `ExcludeMimeTypes` 字段匹配时，**仅检查文件的直接 MIME 类型和别名**，不包括父类型

**重要**：由于父类型自动匹配机制，某些文件格式可能意外匹配到父类型，使用 `ExcludeMimeTypes` 可以排除这些情况。

**示例**：
```ini
# 仅对图片文件显示
MimeType=image/*;

# 排除压缩文件（注意：xlsx等Office文件的父类型是zip，也会被排除）
X-DFM-ExcludeMimeTypes=application/zip;application/x-tar;

# 更精确的排除：只排除真正的压缩文件，不排除Office文件
X-DFM-ExcludeMimeTypes=application/x-compress;application/x-gzip;application/x-bzip2;
```

#### URL 方案过滤字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `X-DDE-FileManager-SupportSchemes` | 字符串列表 | 否 | 支持的 URL 方案，分号分隔 |
| `X-DFM-SupportSchemes` | 字符串列表 | 否 | 同上，别名字段 |

**常见 URL 方案**：

| 方案 | 说明 |
|------|------|
| `file` | 本地文件系统 |
| `ftp` | FTP 协议 |
| `sftp` | SFTP 协议 |
| `smb` | Samba 网络共享 |
| `recent` | 最近访问 |
| `trash` | 回收站 |
| `search` | 搜索结果 |

**示例**：
```ini
# 仅对本地文件显示
X-DFM-SupportSchemes=file;
```

#### 文件后缀过滤字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `X-DDE-FileManager-SupportSuffix` | 字符串列表 | 否 | 支持的文件后缀，分号分隔 |
| `X-DFM-SupportSuffix` | 字符串列表 | 否 | 同上，别名字段 |

**后缀匹配规则**：
- 支持通配符：`7z.*` 可以匹配 `7z.001`, `7z.002`, `7z.003` 等
- **特殊处理**：当选中多个文件都是 7z 分卷文件（7z.001, 7z.002...）时，会作为一组处理

**示例**：
```ini
# 支持所有7z分卷压缩文件（推荐使用通配符）
X-DFM-SupportSuffix=7z.*;

# 或者明确指定具体后缀
X-DFM-SupportSuffix=.7z.001;.7z.002;.7z.003;
```

#### 场景过滤字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `X-DDE-FileManager-NotShowIn` | 字符串列表 | 否 | 不显示的场景，分号分隔 |
| `X-DFM-NotShowIn` | 字符串列表 | 否 | 同上，别名字段 |

**NotShowIn 可选值**：

| 值 | 说明 |
|----|------|
| `Desktop` | 在桌面环境中不显示 |
| `Filemanager` | 在文件管理器中不显示 |

**示例**：
```ini
# 仅在文件管理器中显示，桌面不显示
X-DFM-NotShowIn=Desktop;
```

#### 子菜单字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `Actions` | 字符串列表 | 否 | 子菜单动作标识列表，分号分隔 |

**示例**：
```ini
Actions=CompressHigh;CompressMedium;CompressLow;
```

### Desktop Action ActionName 组字段

子菜单配置组，每个动作对应一个子菜单项。

| 字段     | 类型  | 必需    | 说明        |
| ------ | --- | ----- | --------- |
| `Name` | 字符串 | **是** | 子菜单项显示名称  |
| `Icon` | 字符串 | 否     | 子菜单项图标    |
| `Exec` | 字符串 | **是** | 子菜单项执行的命令 |

**示例**：
```ini
[Desktop Action CompressHigh]
Name=High Compression
Icon=compress-high
Exec=/usr/bin/my-compress --level=high %F
```

### 命令参数占位符

`Exec` 字段支持以下参数占位符，执行时自动替换：

| 占位符 | 说明 | 替换内容 | 适用场景 |
|-------|------|---------|---------|
| `%p` | 当前目录路径 | 本地文件绝对路径 | EmptyArea 菜单 |
| `%f` | 单个文件路径 | 本地文件绝对路径 | SingleFile/SingleDir 菜单 |
| `%F` | 多个文件路径 | 本地文件绝对路径，展开为多个独立参数 | MultiFileDirs 菜单 |
| `%u` | 单个文件 URL | 完整 URL 字符串 | SingleFile/SingleDir 菜单 |
| `%U` | 多个文件 URL | 完整 URL 字符串，展开为多个独立参数 | MultiFileDirs 菜单 |

**参数替换规则**：

1. **单参数替换** (`%p`, `%f`, `%u`)：
   - 在命令参数中查找占位符
   - 占位符可以嵌入在其他字符串中，如 `--file=%f`
   - 支持在引号内使用：`"处理文件: %f"`

2. **列表参数替换** (`%F`, `%U`)：
   - **占位符必须作为独立参数**，不能嵌入其他字符串
   - 替换时会展开为多个独立参数，每个文件作为一个参数
   - 例如：`cmd %F` → `cmd file1 file2 file3`
   - **错误用法**：`cmd --files=%F`（不会正确替换）

3. **引号和空格处理**：
   - 命令解析支持单引号和双引号
   - 引号内的空格不作为参数分隔符
   - 包含空格的文件路径会被自动正确处理

4. **执行顺序**：
   - 第一个参数作为可执行程序路径
   - 其余参数按顺序传递给程序
   - 多次出现的占位符会按从左到右的顺序依次替换

**替换示例**：

```bash
# 示例1：单文件路径替换（嵌入式）
Exec=/usr/bin/tool --file=%f --output=/tmp
# 选中文件：/home/user/My Document.txt
# 替换后：
/usr/bin/tool --file=/home/user/My Document.txt --output=/tmp

# 示例2：单文件路径替换（引号内）
Exec=/usr/bin/echo "Processing: %f"
# 选中文件：/home/user/test.txt
# 替换后：
/usr/bin/echo "Processing: /home/user/test.txt"

# 示例3：多文件路径替换（独立参数）
Exec=/usr/bin/process %F
# 选中文件：file1.txt, file2.txt, file3.txt
# 替换后：
/usr/bin/process file1.txt file2.txt file3.txt

# 示例4：多文件URL替换
Exec=/usr/bin/open %U
# 选中文件：file:///tmp/test1.txt, ftp://server/file.txt
# 替换后：
/usr/bin/open /tmp/test1.txt ftp://server/file.txt

# 示例5：多个占位符
Exec=/usr/bin/compressor --input=%f --output=%p/compressed
# 当前目录：/home/user
# 选中文件：/home/user/data.txt
# 替换后：
/usr/bin/compressor --input=/home/user/data.txt --output=/home/user/compressed

# 错误示例：%F不能嵌入使用
Exec=/usr/bin/tool --files=%F  # 不会正确替换！
# 正确写法：
Exec=/usr/bin/tool --files %F  # 或者使用脚本包装
```

## 配置示例

### 示例1：空白区域菜单

在空白区域添加"打开终端"菜单项：

```ini
[Desktop Entry]
Type=Application
Name=Open Terminal Here
Name[zh_CN]=在此打开终端
Icon=utilities-terminal
Comment=Open a terminal in current directory
Exec=x-terminal-emulator -w %p
X-DFM-MenuTypes=EmptyArea;
X-DFM-SupportSchemes=file;
```

### 示例2：单文件菜单

为文本文件添加"用记事本编辑"菜单项：

```ini
[Desktop Entry]
Type=Application
Name=Edit with Notepad
Name[zh_CN]=用记事本编辑
Icon=text-editor
Exec=/usr/bin/gedit %f
X-DFM-MenuTypes=SingleFile;
MimeType=text/*;
X-DFM-SupportSchemes=file;
```

### 示例3：多选文件菜单

为选中的文件添加批量重命名功能：

```ini
[Desktop Entry]
Type=Application
Name=Batch Rename
Name[zh_CN]=批量重命名
Icon=edit-rename
Exec=/usr/bin/my-rename-tool %F
X-DFM-MenuTypes=MultiFileDirs;
X-DFM-SupportSchemes=file;
```

### 示例4：带子菜单的图片工具

展示如何为图片文件创建多级子菜单，提供常用的图片处理功能：

```ini
[Desktop Entry]
Type=Application
Name=Image Tools
Name[zh_CN]=图片工具
Name[zh_TW]=圖片工具
Icon=image-x-generic
Comment=Quick image processing tools
Comment[zh_CN]=快速图片处理工具
X-DFM-MenuTypes=SingleFile;MultiFileDirs;
X-DFM-SupportSchemes=file;
MimeType=image/jpeg;image/png;image/gif;image/bmp;
X-DFM-ExcludeMimeTypes=image/svg+xml;
Actions=Rotate90;Rotate180;ConvertJPG;ConvertPNG;

[Desktop Action Rotate90]
Name=Rotate 90° Clockwise
Name[zh_CN]=顺时针旋转 90°
Icon=object-rotate-right
Exec=/usr/bin/image-processor --rotate 90 %F

[Desktop Action Rotate180]
Name=Rotate 180°
Name[zh_CN]=旋转 180°
Icon=object-rotate-right
Exec=/usr/bin/image-processor --rotate 180 %F

[Desktop Action ConvertJPG]
Name=Convert to JPG
Name[zh_CN]=转换为 JPG
Icon=image-jpeg
Exec=/usr/bin/image-processor --format jpg --output=%p/converted %F

[Desktop Action ConvertPNG]
Name=Convert to PNG
Name[zh_CN]=转换为 PNG
Icon=image-png
Exec=/usr/bin/image-processor --format png --output=%p/converted %F
```

### 示例5：条件过滤组合

仅对本地图片文件显示的菜单：

```ini
[Desktop Entry]
Type=Application
Name=Image Converter
Name[zh_CN]=图片格式转换
Icon=image-x-generic
Exec=/usr/bin/image-convert %F
X-DFM-MenuTypes=SingleFile;MultiFileDirs;
MimeType=image/*;
X-DFM-ExcludeMimeTypes=image/svg+xml;image/x-xcf;
X-DFM-SupportSchemes=file;
X-DFM-NotShowIn=Desktop;
```

## 部署说明

### 文件部署

#### 方式一：系统级部署（直接加载）

**适用场景**：系统预装、不允许用户修改的菜单

```bash
# 复制配置文件到系统目录
sudo cp myapp-menu.desktop /usr/etc/deepin/menu-extensions/

# 或者
sudo cp myapp-menu.desktop /etc/deepin/menu-extensions/
```

#### 方式二：系统共享部署（推荐）

**适用场景**：软件包安装、OEM 定制、需要分发给所有用户且允许用户自定义的菜单

```bash
# 部署到系统共享目录
sudo cp myapp-menu.desktop /usr/share/deepin/dde-file-manager/oem-menuextensions/
```

#### 方式三：用户级部署

**适用场景**：单用户安装、开发测试、个人自定义菜单

```bash
# 创建用户目录（如不存在）
mkdir -p ~/.local/share/deepin/dde-file-manager/oem-menuextensions

# 复制配置文件
cp myapp-menu.desktop ~/.local/share/deepin/dde-file-manager/oem-menuextensions/
```

### 卸载方式

#### 卸载直接加载的系统配置

```bash
# 删除直接加载的系统级配置
sudo rm /usr/etc/deepin/menu-extensions/myapp-menu.desktop
sudo rm /etc/deepin/menu-extensions/myapp-menu.desktop
```

#### 卸载系统共享目录配置（自动同步清理）

```bash
# 删除系统共享目录的配置文件
sudo rm /usr/share/deepin/dde-file-manager/oem-menuextensions/myapp-menu.desktop
```

#### 卸载用户级配置

```bash
# 删除用户级配置
rm ~/.local/share/deepin/dde-file-manager/oem-menuextensions/myapp-menu.desktop

# 如果需要清空整个目录
rm -rf ~/.local/share/deepin/dde-file-manager/oem-menuextensions/
```

### 过滤规则执行顺序

菜单项过滤按以下顺序执行（任一规则不通过则移除菜单项）：

#### 第一阶段：MenuType 筛选
1. **MenuTypes 匹配**：根据当前场景类型（EmptyArea/SingleFile/SingleDir/MultiFileDirs）筛选出对应的菜单项列表

#### 第二阶段：逐项过滤
对第一阶段筛选出的每个菜单项，按以下顺序检查：

2. **NotShowIn 检查**：检查当前场景（Desktop/Filemanager）是否在排除列表中
   - 如果 `X-DFM-NotShowIn` 字段不存在，则通过检查
   - 如果存在且包含当前场景，则移除此菜单项

3. **SupportSchemes 检查**：检查文件/目录的 URL 方案（file/ftp/smb等）是否支持
   - 如果 `X-DFM-SupportSchemes` 字段不存在，则通过检查
   - 如果存在，必须匹配才能通过

4. **SupportSuffix 检查**：检查文件后缀是否匹配（仅对文件，目录跳过）
   - 如果 `X-DFM-SupportSuffix` 字段不存在，则通过检查
   - 如果存在，必须匹配才能通过
   - 特殊处理：多选文件都是 7z 分卷文件时作为一组处理

5. **ExcludeMimeTypes 检查**：检查文件的 **直接 MIME 类型**（不含父类型）是否在排除列表中
   - 如果在排除列表中，则移除此菜单项
   - **注意**：此检查**不考虑父类型**，只检查文件的直接 MIME 类型和别名

6. **MimeType 匹配**：检查文件的 **完整 MIME 类型列表**（含父类型）是否匹配支持列表
   - 如果 `MimeType` 字段不存在，则通过检查（相当于 `MimeType=*`）
   - 如果存在，必须匹配才能通过
   - **注意**：此检查**考虑父类型**，会匹配文件的所有父 MIME 类型

#### 第三阶段：特殊规则
7. **特殊情况处理**：
   - **FTP 压缩禁用**：对于 FTP 协议的文件，名为 "Compress" 的菜单项会被自动移除
   - **MTP 设备特殊处理**：对于 MTP 设备挂载的目录，`application/octet-stream` 类型的匹配会被忽略

**过滤流程图**：
```
所有菜单项配置
     ↓
[MenuType 筛选] → 获取对应场景的菜单项
     ↓
[NotShowIn 检查] → Desktop/Filemanager 场景过滤
     ↓
[SupportSchemes 检查] → URL 方案过滤
     ↓
[SupportSuffix 检查] → 文件后缀过滤
     ↓
[ExcludeMimeTypes 检查] → 排除特定 MIME 类型（不含父类型）
     ↓
[MimeType 匹配] → MIME 类型匹配（含父类型）
     ↓
[特殊规则] → FTP/MTP 等特殊处理
     ↓
最终显示的菜单项
```

**重要提示**：
- ExcludeMimeTypes 和 MimeType 的父类型处理逻辑**不同**，建议优先使用 ExcludeMimeTypes 排除不需要的类型
- 多选文件时，每个文件都会单独检查，只要有一个文件不匹配，该菜单项就会被移除


## 注意事项

### 性能优化

#### 避免过度匹配

不良示例：
```ini
# 对所有文件显示，会导致菜单冗余
X-DFM-MenuTypes=SingleFile;SingleDir;MultiFileDirs;
```

推荐：
```ini
# 仅对特定文件类型显示
X-DFM-MenuTypes=SingleFile;
MimeType=text/plain;
```

#### 子菜单合理使用

不良示例：
```ini
# 大量子菜单会降低用户体验
Actions=Action1;Action2;Action3;...;Action20;
```

推荐：
```ini
# 合理分组，不超过 5-7 个子菜单
Actions=CommonAction;AdvancedAction;
```

### 安全性

#### 命令注入防护

**危险示例**：
```ini
# 不要直接拼接用户输入到 shell 命令
Exec=bash -c "rm -rf %f"
```

**安全示例**：
```ini
# 使用专用工具处理文件，避免 shell 解释
Exec=/usr/bin/my-safe-tool %f
```

#### 权限控制

- OEM 菜单执行的命令拥有与文件管理器相同的权限（用户权限）
- 不要在菜单中执行需要 root 权限的操作
- 如需提权，使用 `pkexec` 或 `sudo`（需配置 sudoers）

#### 路径验证

在脚本中验证路径的有效性：

```bash
#!/bin/bash

file_path="$1"

# 检查文件是否存在
if [ ! -e "$file_path" ]; then
    echo "文件不存在: $file_path"
    exit 1
fi

# 检查文件类型
if [ ! -f "$file_path" ]; then
    echo "不是常规文件: $file_path"
    exit 1
fi

# 处理文件
process_file "$file_path"
```

### 调试方法

#### 验证配置文件

检查 Desktop Entry 格式是否正确：

```bash
# 使用 desktop-file-validate 工具
desktop-file-validate myapp-menu.desktop
```

#### 查看日志

文件管理器日志输出：

```bash
# 查看文件管理器日志
journalctl --user-unit dde-file-manager -f
```

#### 命令测试

手动测试命令替换是否正确：

```bash
# 模拟执行
/usr/bin/my-tool /path/to/test/file.txt
```

### 常见问题

#### 1. 菜单不显示

**可能原因**：
- Desktop Entry 格式错误
- MenuTypes 未设置或设置错误
- 过滤条件过于严格

**排查步骤**：
```bash
# 1. 验证配置文件
desktop-file-validate myapp-menu.desktop

# 2. 检查文件权限
ls -la /etc/deepin/menu-extensions/myapp-menu.desktop

# 3. 查看文件管理器日志
journalctl -f | grep dde-file-manager
```

#### 2. 子菜单不显示

**可能原因**：
- `Actions` 字段格式错误
- 子配置组命名不匹配
- 子配置组缺少必需字段

**正确示例**：
```ini
[Desktop Entry]
Type=Application
Actions=Action1;Action2;

[Desktop Action Action1]
Name=Sub Menu 1
Exec=/usr/bin/tool1

[Desktop Action Action2]
Name=Sub Menu 2
Exec=/usr/bin/tool2
```

#### 3. 命令执行失败

**可能原因**：
- 命令路径不存在
- 缺少执行权限
- 参数替换错误

**排查步骤**：
```bash
# 1. 检查命令是否存在
which my-command

# 2. 检查执行权限
ls -la /usr/bin/my-command

# 3. 手动执行测试
/usr/bin/my-command --test
```

#### 4. 参数替换不正确

**可能原因**：
- 占位符拼写错误
- 引号处理错误

**调试方法**：

在脚本中打印参数：
```bash
#!/bin/bash
echo "参数数量: $#" >> /tmp/menu-debug.log
echo "参数内容: $@" >> /tmp/menu-debug.log
```

#### 5. 图标不显示

**可能原因**：
- 图标名称错误
- 图标文件不存在

**解决方法**：
```bash
# 查找系统中可用的图标
find /usr/share/icons -name "*compress*"

# 或使用标准图标名称
Icon=application-x-compress
```

### 已知限制和特殊情况

#### 1. MTP 设备目录特殊处理
- **问题描述**：某些 MTP 挂载设备的目录 MIME 类型不符合规范（被识别为 `application/octet-stream` 而非 `inode/directory`）
- **特殊处理**：对于路径包含 `/mtp:host` 的文件，`application/octet-stream` 的匹配会被忽略
- **影响范围**：通过 MTP 协议连接的移动设备和相机

#### 2. 配置文件格式要求
- Desktop Entry 文件必须包含 `Type=Application` 字段（Desktop Entry 规范要求）
- 文件扩展名必须为 `.desktop`
- 文件编码必须为 UTF-8
- `X-DFM-MenuTypes` 字段为必需，且必须包含有效值

#### 3. 父 MIME 类型匹配的副作用
- **问题**：某些文件格式的父类型可能导致意外匹配
  - 例如：Office 文档（xlsx, docx）的父类型是 `application/zip`
  - 如果 `MimeType=application/zip`，会同时匹配压缩文件和 Office 文档
- **解决方案**：使用 `ExcludeMimeTypes` 排除不需要的类型，或使用更具体的 MIME 类型

```ini
# 示例：只支持纯压缩文件，不包括 Office 文档
MimeType=application/zip;application/x-tar;
X-DFM-ExcludeMimeTypes=application/vnd.openxmlformats-officedocument.*;

# 或者使用更具体的 MIME 类型
MimeType=application/x-compress;application/x-gzip;
```

#### 6. 命令执行环境
- 命令以文件管理器的用户权限执行（非 root）
- 命令在后台通过 fork/exec 执行，不会阻塞 UI
- 标准输出和标准错误输出不会显示给用户（建议脚本自行处理输出）

### 最佳实践

1. **配置文件命名**：使用供应商前缀避免冲突（如 `mycompany-feature.desktop`）

2. **本地化支持**：为常用语言提供翻译
   ```ini
   Name=My Action
   Name[zh_CN]=我的操作
   Name[zh_TW]=我的操作
   Name[de]=Meine Aktion
   ```

3. **MIME 类型精确匹配**：优先使用具体的 MIME 类型，避免过于宽泛的匹配
   ```ini
   # 推荐：明确指定支持的图片格式
   MimeType=image/jpeg;image/png;image/gif;
   
   # 不推荐：过于宽泛，可能匹配不需要的类型
   MimeType=image/*;
   ```

4. **ExcludeMimeTypes 优先使用**：由于不检查父类型，更适合排除特定文件
   ```ini
   # 排除压缩文件，但不影响 Office 文档
   X-DFM-ExcludeMimeTypes=application/x-compress;application/x-tar;application/x-bzip2;
   ```

5. **参数占位符正确使用**：
   - 单个参数用 `%f`/`%u`，可以嵌入
   - 多个参数用 `%F`/`%U`，必须独立
   ```ini
   # 正确：单个文件路径嵌入
   Exec=/usr/bin/tool --file=%f
   
   # 正确：多个文件路径独立参数
   Exec=/usr/bin/tool %F
   
   # 错误：多个文件路径嵌入（不会正确替换）
   Exec=/usr/bin/tool --files=%F
   ```

6. **脚本错误处理**：在脚本中添加完善的错误检查
   ```bash
   #!/bin/bash
   # 检查参数
   if [ $# -eq 0 ]; then
       notify-send "错误" "没有提供文件参数"
       exit 1
   fi
   
   # 检查文件存在性
   for file in "$@"; do
       if [ ! -e "$file" ]; then
           notify-send "错误" "文件不存在: $file"
           exit 1
       fi
   done
   
   # 执行操作...
   ```

7. **日志记录**：脚本执行时记录关键信息便于调试
   ```bash
   #!/bin/bash
   LOG_FILE="$HOME/.cache/my-menu-action.log"
   echo "[$(date)] 执行操作，参数: $@" >> "$LOG_FILE"
   ```

8. **权限最小化**：仅请求必需的权限，避免使用 sudo

9. **用户确认**：对危险操作提供确认对话框
   ```bash
   if zenity --question --text="确定要删除这些文件吗？"; then
       # 执行删除操作
   fi
   ```

10. **路径处理**：正确处理包含空格和特殊字符的路径
    ```bash
    for file in "$@"; do
        # 使用引号保护变量
        process_file "$file"
    done
    ```

11. **测试覆盖**：在不同场景下充分测试
    - 单个文件/目录
    - 多个文件/目录混合选择
    - 包含空格和特殊字符的文件名
    - 不同 MIME 类型的文件
    - 桌面和文件管理器两种场景
    - 不同 URL 方案（file, ftp, smb 等）

12. **性能考虑**：避免在脚本中执行耗时操作
    ```bash
    # 对于耗时操作，在后台执行并立即返回
    (long_running_operation "$@" &)
    ```

## 附录

### 相关规范

- [Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/)
- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/)
- [Icon Theme Specification](https://specifications.freedesktop.org/icon-theme-spec/)

### 术语表

| 术语 | 说明 |
|------|------|
| Desktop Entry | freedesktop.org 定义的桌面配置文件格式 |
| MIME Type | 文件类型标识符（Multipurpose Internet Mail Extensions） |
| URL Scheme | URL 方案，如 file、ftp、http 等 |
| OEM | Original Equipment Manufacturer，原始设备制造商 |
| Fork | Unix 系统中创建子进程的系统调用 |
| Exec | Desktop Entry 中指定执行命令的字段 |

