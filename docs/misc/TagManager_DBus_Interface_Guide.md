# TagManager DBus 接口使用文档

## 概述

本文档介绍 dde-file-manager daemon 中 `org.deepin.Filemanager.Daemon.TagManager` DBus 接口的使用方法，面向测试人员，所有示例使用 `gdbus` 命令行工具演示。

---

## 仓库与代码基线

- 仓库：`https://github.com/linuxdeepin/dde-file-manager`
- 关键代码位置：
  - DBus 服务注册：`src/plugins/daemon/tag/tagdaemon.cpp`
  - DBus 方法实现：`src/plugins/daemon/tag/tagmanagerdbus.h` / `.cpp`
  - 数据库操作层：`src/plugins/daemon/tag/tagdbhandler.h` / `.cpp`
  - 操作码枚举定义：`src/plugins/daemon/tag/daemonplugin_tag_global.h`
  - DBus introspection XML：`assets/dbus/org.deepin.Filemanager.Daemon.TagManager.xml`
  - 客户端调用封装：`src/plugins/common/dfmplugin-tag/data/tagproxyhandle.cpp`

---

## DBus 连接信息

| 项目 | 值 |
|---|---|
| Bus | Session Bus（会话总线） |
| Service Name | `org.deepin.Filemanager.Daemon` |
| Object Path | `/org/deepin/Filemanager/Daemon/TagManager` |
| Interface | `org.deepin.Filemanager.Daemon.TagManager` |

---

## 快速开始

设置公共环境变量（后续所有示例均假设已执行）：

```bash
S="org.deepin.Filemanager.Daemon"
O="/org/deepin/Filemanager/Daemon/TagManager"
I="org.deepin.Filemanager.Daemon.TagManager"
```

---

## 操作码枚举

所有 method 的第一个参数 `opt` 是一个 int，对应以下枚举（定义于 `daemonplugin_tag_global.h`，从 0 开始）：

### QueryOpts（Query 方法的 opt）

| 值 | 枚举名 | 含义 |
|---|---|---|
| 0 | `kTags` | 获取所有标签 |
| 1 | `kFilesWithTags` | 获取所有带标签的文件 |
| 2 | `kTagsOfFile` | 获取指定文件的标签 |
| 3 | `kFilesOfTag` | 获取指定标签下的文件 |
| 4 | `kColorOfTags` | 获取指定标签的颜色 |
| 5 | `kTagIntersectionOfFiles` | 获取多个文件的公共标签 |
| 6 | `kTrashFileTags` | 查询回收站文件的标签 |
| 7 | `kAllTrashFileTags` | 获取所有回收站文件标签 |

### InsertOpts（Insert 方法的 opt）

| 值 | 枚举名 | 含义 |
|---|---|---|
| 0 | `kTags` | 创建新标签 |
| 1 | `kTagOfFiles` | 给文件打标签 |
| 2 | `kTrashFileTags` | 保存回收站文件标签 |

### DeleteOpts（Delete 方法的 opt）

| 值 | 枚举名 | 含义 |
|---|---|---|
| 0 | `kTags` | 删除标签 |
| 1 | `kFiles` | 删除文件的所有标签记录 |
| 2 | `kTagOfFiles` | 移除文件的指定标签 |
| 3 | `kTrashFileTags` | 删除回收站文件标签 |
| 4 | `kAllTrashTags` | 清除所有回收站标签 |

### UpdateOpts（Update 方法的 opt）

| 值 | 枚举名 | 含义 |
|---|---|---|
| 0 | `kColors` | 修改标签颜色 |
| 1 | `kTagsNameWithFiles` | 修改标签名称 |
| 2 | `kFilesPaths` | 修改文件路径 |

---

## 系统支持的颜色名称

标签颜色以字符串存储，系统内置颜色名（定义于 `src/plugins/common/dfmplugin-tag/utils/taghelper.cpp`）：

| 颜色名 | 十六进制值 |
|---|---|
| `Orange` | `#ffa503` |
| `Red` | `#ff1c49` |
| `Purple` | `#9023fc` |
| `Navy-blue` | `#3468ff` |
| `Azure` | `#00b5ff` |
| `Grass-green` | `#58df0a` |
| `Yellow` | `#fef144` |
| `Gray` | `#cccccc` |

---

## DBus Method 签名详解

### 1. Query — 查询

DBus 签名有两个重载：
- `Query(out v, in i opt)` — 不带 value 参数
- `Query(out v, in i opt, in as value)` — 带 value 参数（QStringList）

返回值为 `QDBusVariant`（DBus 类型 `v`），内部包装的具体类型因 opt 而异。

#### 1.1 查询所有标签（opt=0, kTags）

- **参数**：opt=0，无需 value
- **返回**：`a{sv}` — 键为标签名，值为颜色字符串
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 0
```
- **返回示例**：`(<{'red': <'Red'>, 'work': <'Orange'>}>,)`

#### 1.2 查询所有带标签的文件（opt=1, kFilesWithTags）

- **参数**：opt=1，无需 value
- **返回**：`a{sv}` — 键为文件路径，值为标签名 QStringList
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 1
```
- **返回示例**：`(<{'/home/user/file.txt': <['red', 'work']>}>,)`

#### 1.3 查询指定文件的标签（opt=2, kTagsOfFile）

- **参数**：opt=2，value=文件路径列表
- **返回**：`a{sv}` — 键为文件路径，值为该文件的标签列表
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 2 "['/home/user/file1.txt', '/home/user/file2.txt']"
```

#### 1.4 查询指定标签下的文件（opt=3, kFilesOfTag）

- **参数**：opt=3，value=标签名列表
- **返回**：`a{sv}` — 键为标签名，值为文件路径列表
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 3 "['red', 'work']"
```

#### 1.5 查询指定标签的颜色（opt=4, kColorOfTags）

- **参数**：opt=4，value=标签名列表
- **返回**：`a{sv}` — 键为标签名，值为颜色字符串
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 4 "['red', 'work']"
```

#### 1.6 查询多个文件的公共标签（opt=5, kTagIntersectionOfFiles）

- **参数**：opt=5，value=文件路径列表
- **返回**：`as` — 所有文件共有的标签名列表
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 5 "['/home/user/file1.txt', '/home/user/file2.txt']"
```

#### 1.7 查询回收站文件标签（opt=6, kTrashFileTags）

- **参数**：opt=6，value=`["originalPath:<原始路径>", "inode:<inode号>"]`
- **返回**：`a{sv}` — `{"tags": <标签列表>}`
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 6 "['originalPath:/home/user/file.txt', 'inode:123456']"
```

#### 1.8 查询所有回收站文件标签（opt=7, kAllTrashFileTags）

- **参数**：opt=7，无需 value
- **返回**：`a{sv}` — 键为 `"原始路径:inode"`，值为标签列表
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Query 7
```

---

### 2. Insert — 新增

DBus 签名：`Insert(out b, in i opt, in a{sv} value)`
返回 `bool`，表示操作是否成功。

#### 2.1 创建新标签（opt=0, kTags）

- **参数**：opt=0，value=`a{sv}` — 键为标签名，值为颜色字符串
- **行为**：向 `tag_property` 表插入新标签属性；若标签已存在则跳过。触发 `NewTagsAdded` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Insert 0 "{'red': <'Red'>, 'work': <'Orange'>}"
```

#### 2.2 给文件打标签（opt=1, kTagOfFiles）

- **参数**：opt=1，value=`a{sv}` — 键为文件路径，值为标签名 QStringList
- **行为**：向 `file_tags` 表插入文件-标签映射，自动去重。触发 `FilesTagged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Insert 1 "{'/home/user/file.txt': <['red', 'work']>}"
```

#### 2.3 保存回收站文件标签（opt=2, kTrashFileTags）

- **参数**：opt=2，value=`a{sv}`，包含三个键：
  - `"originalPath"`：文件原始路径（string）
  - `"inode"`：文件 inode 号（int64）
  - `"tags"`：标签列表（QStringList）
- **行为**：向 `trash_file_tags` 表插入记录。触发 `TrashFileTagsChanged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Insert 2 "{'originalPath': <'/home/user/file.txt'>, 'inode': <123456>, 'tags': <['red', 'work']>}"
```

---

### 3. Delete — 删除

DBus 签名：`Delete(out b, in i opt, in a{sv} value)`
返回 `bool`。

#### 3.1 删除标签（opt=0, kTags）

- **参数**：opt=0，value=`a{sv}`，其中**第一个值**为要删除的标签名 QStringList（键名任意，daemon 取 `value.first().toStringList()`）
- **行为**：从 `tag_property` 表和 `file_tags` 表中同时删除该标签及其所有关联记录。触发 `TagsDeleted` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Delete 0 "{'tags': <['red', 'work']>}"
```

#### 3.2 删除文件的所有标签记录（opt=1, kFiles）

- **参数**：opt=1，value=`a{sv}`，**键**为要删除的文件路径（值任意，daemon 取 `value.keys()`）
- **行为**：从 `file_tags` 表中删除这些文件的所有标签记录。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Delete 1 "{'/home/user/file1.txt': <''>, '/home/user/file2.txt': <''>}"
```

#### 3.3 移除文件的指定标签（opt=2, kTagOfFiles）

- **参数**：opt=2，value=`a{sv}` — 键为文件路径，值为要移除的标签名 QStringList
- **行为**：从 `file_tags` 表中删除指定文件-标签的映射记录。触发 `FilesUntagged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Delete 2 "{'/home/user/file.txt': <['red']>}"
```

#### 3.4 删除回收站文件标签（opt=3, kTrashFileTags）

- **参数**：opt=3，value=`a{sv}`，包含：
  - `"originalPath"`：文件原始路径（string）
  - `"inode"`：文件 inode 号（int64）
- **行为**：从 `trash_file_tags` 表删除对应记录。触发 `TrashFileTagsChanged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Delete 3 "{'originalPath': <'/home/user/file.txt'>, 'inode': <123456>}"
```

#### 3.5 清除所有回收站标签（opt=4, kAllTrashTags）

- **参数**：opt=4，value=空 `{}`（可传空 map）
- **行为**：删除 `trash_file_tags` 表中所有记录。触发 `TrashFileTagsChanged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Delete 4 "{}"
```

> **注意**：此接口仅清除回收站标签，不会删除普通标签。如果要删除所有普通标签，需要先通过 `Query 0` 获取所有标签名，再调用 `Delete 0` 传入标签列表。

---

### 4. Update — 修改

DBus 签名：`Update(out b, in i opt, in a{sv} value)`
返回 `bool`。

#### 4.1 修改标签颜色（opt=0, kColors）

- **参数**：opt=0，value=`a{sv}` — 键为标签名，值为新颜色字符串
- **行为**：更新 `tag_property` 表中标签的颜色。触发 `TagsColorChanged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Update 0 "{'red': <'Blue'>}"
```

#### 4.2 修改标签名称（opt=1, kTagsNameWithFiles）

- **参数**：opt=1，value=`a{sv}` — 键为旧标签名，值为新标签名
- **行为**：同时更新 `tag_property` 表和 `file_tags` 表中的标签名。触发 `TagsNameChanged` 信号。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Update 1 "{'oldName': <'newName'>}"
```

#### 4.3 修改文件路径（opt=2, kFilesPaths）

- **参数**：opt=2，value=`a{sv}` — 键为旧路径，值为新路径
- **行为**：更新 `file_tags` 表中的文件路径记录。
- **示例**：
```bash
gdbus call --session --dest $S --object-path $O --method $I.Update 2 "{'/old/path/file.txt': <'/new/path/file.txt'>}"
```

---

## DBus Signals（信号）

| 信号名 | 参数 | 触发时机 |
|---|---|---|
| `TagsServiceReady` | 无 | 服务启动就绪 |
| `NewTagsAdded` | `a{sv} tags` | 创建新标签后 |
| `TagsDeleted` | `as tags` | 删除标签后 |
| `TagsColorChanged` | `a{sv} oldAndNew` | 修改标签颜色后 |
| `TagsNameChanged` | `a{sv} oldAndNew` | 修改标签名称后 |
| `FilesTagged` | `a{sv} fileAndTags` | 给文件打标签后 |
| `FilesUntagged` | `a{sv} fileAndTags` | 移除文件标签后 |
| `TrashFileTagsChanged` | 无 | 回收站标签变化时 |

监听信号示例：
```bash
gdbus monitor --session --dest $S --object-path $O
```

---

## 数据模型与持久化

底层使用 SQLite 数据库（通过 `dfm-base` 的 `SqliteHandle` ORM），涉及三张表：

| 表名 | 用途 | 关键字段 |
|---|---|---|
| `tag_property` | 标签属性 | `tagName`, `tagColor`, `future`, `ambiguity` |
| `file_tags` | 文件-标签关联 | `filePath`, `tagName` |
| `trash_file_tags` | 回收站文件标签 | `originalPath`, `fileInode`, `tagNames`, `trashIndex` |

Bean 定义位于 `src/plugins/daemon/tag/beans/` 目录下。

---

## 完整操作流程示例

以下演示一个完整的标签操作流程，可直接复制执行（需先设置环境变量）：

```bash
# 设置公共变量
S="org.deepin.Filemanager.Daemon"
O="/org/deepin/Filemanager/Daemon/TagManager"
I="org.deepin.Filemanager.Daemon.TagManager"

# 1. 创建标签 "red"（颜色 Red）和 "work"（颜色 Orange）
gdbus call --session --dest $S --object-path $O --method $I.Insert 0 "{'red': <'Red'>, 'work': <'Orange'>}"

# 2. 查询所有标签，确认已创建
gdbus call --session --dest $S --object-path $O --method $I.Query 0

# 3. 给文件打标签
gdbus call --session --dest $S --object-path $O --method $I.Insert 1 "{'/tmp/test.txt': <['red', 'work']>}"

# 4. 查询该文件的标签
gdbus call --session --dest $S --object-path $O --method $I.Query 2 "['/tmp/test.txt']"

# 5. 查询 "red" 标签下的所有文件
gdbus call --session --dest $S --object-path $O --method $I.Query 3 "['red']"

# 6. 修改 "red" 标签颜色为 Blue
gdbus call --session --dest $S --object-path $O --method $I.Update 0 "{'red': <'Blue'>}"

# 7. 修改标签名 "red" -> "important"
gdbus call --session --dest $S --object-path $O --method $I.Update 1 "{'red': <'important'>}"

# 8. 移除文件的 "work" 标签
gdbus call --session --dest $S --object-path $O --method $I.Delete 2 "{'/tmp/test.txt': <['work']>}"

# 9. 删除 "important" 标签（同时删除所有文件的关联）
gdbus call --session --dest $S --object-path $O --method $I.Delete 0 "{'tags': <['important']>}"

# 10. 删除文件的所有标签记录
gdbus call --session --dest $S --object-path $O --method $I.Delete 1 "{'/tmp/test.txt': <''>}"
```

---

## 注意事项

1. **Delete(kTags) 的参数结构特殊**：value 的第一个值（非键）必须是 QStringList，键名任意。这是因为 daemon 端取 `value.first().toStringList()`。
2. **Delete(kFiles) 的参数结构特殊**：value 的键（不是值）是文件路径，值任意。daemon 端取 `value.keys()`。
3. **清除所有普通标签**：没有直接的"清除全部"接口。需先 `Query 0` 获取所有标签名，再 `Delete 0` 传入。`Delete 4`（kAllTrashTags）仅清除回收站标签。
4. **Query 有两个重载**：opt=0/1/7 使用无 value 的 2 参数版本；opt=2/3/4/5/6 使用带 value 的 3 参数版本。gdbus 按参数个数自动匹配。
5. **inode 参数**：回收站相关接口中的 inode 为 int64 类型，gdbus 文本格式中直接写数字即可。
6. **文件路径**：使用本地文件系统绝对路径（如 `/home/user/file.txt`），非 file:// URL。
