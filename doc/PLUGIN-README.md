## 深度文管上下文菜单 OEM 支持插件

在启用此插件支持后，在深度文件管理器载入此插件时，会识别指定目录下的 `.desktop` 文件，并将其视为扩展菜单项，加入到文件管理器的上下文菜单中。

### OEM 方式

每个定制菜单项为一个独立的 Application 类型的 `.desktop` 文件（遵循 [desktop-entry-spec](https://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#extra-actions-identifier)）。对于每个文件，在其 `[Desktop Entry]` 下至少需要包含 `Name` 和 `Exec` 字段，`Name` 的内容为添加到上下文菜单中时将会显示的名称，`Exec` 的内容为当单击此新增的菜单项时将会执行的行为（遵循 desktop 文件规范标准，可以使用 `%u` 等参数）。

定制菜单支持使用 `Actions` 字段增加指定的子菜单，子菜单需至少包含 `Name` 和 `Exec` 字段，可以通过 `Icon` 指定图标。若指定子菜单，则入口项（原本 `[Desktop Entry]` 下）的 `Exec` 字段将不再有效。

对于文件管理器菜单项定制，支持额外的字段可供定制，可以使用 `X-DFM-MenuTypes` 字段来指定菜单项在何种情况会显示，此字段可包含一个或多个类型，其中包括 `SingleFile`, `SingleDir`, `MultiFileDirs` 和 `EmptyArea` 四种。

| 名称 | 含义 |
| :- | :- |
| `SingleFile` | 在单独的文件上触发上下文菜单 |
| `SingleDir` | 在单独的文件夹/目录上触发上下文菜单 |
| `MultiFileDirs` | 在多个文件/文件夹上触发上下文菜单 |
| `EmptyArea`| 在空白区域触发上下文菜单 |

可以使用 `;` 作为分割填写多个值，但注意，包含 `X-DFM-MenuTypes` 字段但内容为空的情况和不包含 `X-DFM-MenuTypes` 字段的情况不同，包含但为空将不会在任何位置显示，不包含将会视为会在任何情况下显示。

`MimeType` 可以根据被选中的文件类型决定菜单项是否显示被添加的项，支持模糊匹配（如 `MimeType=image/*;` ）。 同 `X-DFM-MenuTypes` 一样， `MimeType` 只能用在 `[Desktop Entry]` 项目中，使用 `;` 作为分割填写多个值， `MimeType` 字段但内容为空的情况和不包含 `MimeType` 字段的情况不同，包含但为空将不会在任何位置显示，不包含将会视为 `MimeType=*;`。

`X-DFM-ExcludeMimeTypes` 可以排除特定的文件类型，只能用在 `[Desktop Entry]` 项目中，使用 `;` 作为分割填写多个值，支持模糊匹配。优先于 `MimeTypes`项过滤文件，即如果 `MimeTypes` 和 `X-DFM-ExcludeMimeTypes`同时包含某种文件类型时，该文件类型被选中将无法显示菜单。

注意： MimeType 本身呈树形结构关系，配置 `MimeType` 所对应的值将匹配文件所对应的最佳匹配 MimeType 及其完整父级关系，即若文件的 MimeType 关系树中任意一项匹配了 `MimeType` 中所配置的值，则匹配成功。例如，若配置 `MimeType=text/plain;` 则除 `text/plain` 外还会匹配 `application/json` `application/xml` 等类型的文件，因为它们的父级 MimeType 关系中包含 `text/plain`。 而 `X-DFM-ExcludeMimeTypes` 则仅对完全匹配此配置中所包含的 MimeType 进行过滤，不过滤其父级关系。如 `*.xlxs` 文件的父级 MimeType 有 `application/zip`,但 `ExcludeMimeTypes=application/zip` 不会过滤掉 `*.xlxs` 文件。 

`X-DFM-NotShowIn` 可以决定是否在桌面或者文件管理器显示菜单，此字段可包含一个或多个类型，其中包括 `FileManager`, `Desktop` 两种，字段不存在或字段为空时在桌面和文件管理均可显示。

| 名称 | 含义 |
| :- | :- |
| `FileManager` | 在文件管理器中触发上下文菜单 |
| `Desktop` | 在桌面触发上下文菜单 |

`X-DFM-SupportSchemes` 可以决定是否在指定的目录scheme下显示菜单，此字段可包含一个或多个类型，其中包括 `file`, `trash`, `recent`, `bookmark` 等多种，字段不包含时任何位置均可显示菜单，字段内容为空时将不显示菜单。
| 名称 | 含义 |
| :- | :- |
| `file` | 在本地文件上触发上下文菜单 |
| `trash ` | 在回收站中触发上下文菜单 |
| `recent ` | 在最近文件中触发上下文菜单 |
| `bookmark ` | 在书签文件触发上下文菜单 |


`X-DFM-SupportSuffix`  可以决定是否根据(全)后缀名过滤显示菜单，如在 `test.7z.002` `test.7z.003` `test.7z.004` 等文件上显示压缩菜单，由于分卷文件 `MimeType` 是 `application/octet-stream` 无法通过文件类型显示，特增加通过后缀显示菜单，此情况可以用 `7z.*` 来支持。


OEM 厂商需要将待添加的 `desktop` 文件放置到 `/usr/share/deepin/dde-file-manager/oem-menuextensions/` 位置，在下次启动文件管理器时[^1]，选中任意一个或多个文件并触发上下文菜单，将可以看到新增的项目位于其中。

### 示例 `.desktop` 文件

可以将下面的示例存储为 `test.desktop` 并放置到 `/usr/share/deepin/dde-file-manager/oem-menuextensions/` 中，关闭现有的所有文件管理器窗口并打开新的文件管理器窗口，定制的菜单将会出现在单独目录或多选文件的情况的上下文菜单中。

### 示例 1 - 单项菜单

``` ini
[Desktop Entry]
Type=Application
Exec=/home/wzc/Temp/test.sh %U
Icon=utilities-terminal
Name=示例菜单项名称
X-DFM-MenuTypes=SingleDir;MultiFileDirs;
```

### 示例 2 - 包含子菜单

注意这种情况时，由于包含子菜单，故在 `[Desktop Entry]` 下添加 `Exec` 不会起任何作用。另外，`X-DFM-MenuTypes` 只能用在 `[Desktop Entry]` 项目中，不能用在子菜单中。

``` ini
[Desktop Entry]
Type=Application
Icon=utilities-terminal
Name=示例菜单项名称
Actions=TestAction;
X-DFM-MenuTypes=SingleDir;MultiFileDirs;

[Desktop Action TestAction]
Name=示例子菜单
Exec=/home/wzc/Temp/test.sh %U
```

### 完整示例 - 复制文件路径

1. 切换到 `.desktop` 文件所在目录 `/usr/share/deepin/dde-file-manager/oem-menuextensions/` 。
```
sudo mkdir /usr/share/deepin/dde-file-manager
sudo mkdir /usr/share/deepin/dde-file-manager/oem-menuextensions
cd /usr/share/deepin/dde-file-manager/oem-menuextensions
```
2. 使用文本编辑器（如 `deepin-editor` 或 `vim` ）创建一个文本文件（需要root权限保存）, 名称可以自定义， 后缀名为`.desktop`。
```
sudo deepin-editor copyfilepath.desktop
```
3. 在文件中加入以下内容， `Icon` 图标可以去 `/usr/share/icons/` 中选择合适的图标。 
```
[Desktop Entry]
Type=Application
Icon=edit-copy
Name=复制文件路径
X-DFM-MenuTypes=SingleFile;
Exec=~/copyfilepath.sh %u
MimeType=text/plain;
X-DFM-NotShowIn=Desktop
X-DFM-ExcludeMimeTypes=application/xml;
```
4. 在主目录下创建 `copyfilepath.sh` 脚本。
```
cd ~
deepin-editor copyfilepath.sh
```
在文本中加入以下内容
```
#!/bin/bash
echo -n $1 | xclip -i -selection clipboard
```
至此，重新启动文件管理器，文件列表中常规文本文件的右键菜单会多一个菜单项 `复制文件路径`，点击即可将文件路径复制到剪切板中。

[^1]: 注：这是由于菜单列表仅在文件管理器（桌面）启动时识别并记录一次。在部分平台，深度文件管理器存在加速启动用的进程常驻后台，您可能需要结束所有文件管理器进程。类似的，对于桌面，也需要重启桌面的相应进程才能使桌面使用新增的菜单。
