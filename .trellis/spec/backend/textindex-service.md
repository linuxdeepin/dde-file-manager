# TextIndex 服务技术规范

> DDE File Manager 文本索引服务 - 基于 Lucene++ 的全文检索后台服务。

---

## 概述

TextIndex 服务是 DDE File Manager 的独立文件文本索引服务，提供基于 Lucene++ 的全文检索能力。服务采用分层架构设计，通过 DBus 提供跨进程接口，支持实时文件系统监控和增量索引更新。

**关键特性**:
- 实时文件系统监控（基于 inotify）
- 批量事件收集和处理
- 灵活的任务队列管理
- 支持多种文件类型和编码
- 资源消耗控制（CPU、inotify watches）
- 静默后台索引更新
- 索引损坏自动恢复

---

## 技术栈

| 依赖 | 版本/类型 | 用途 |
|------|-----------|------|
| Qt6 | Core, DBus, Gui | 基础框架 |
| Dtk6::Core | DFileSystemWatcher | 文件系统监控 |
| dfm6-search | - | 全文检索基础 |
| Lucene++ | liblucene++, liblucene++-contrib | 全文检索引擎 |
| docparser | - | 文档解析 |
| glib-2.0 | - | 底层工具 |

---

## 目录结构

```
textindex/
├── fsmonitor/              # 文件系统监控模块
│   ├── fseventcollector.*h    # 事件收集器
│   ├── fseventcontroller.*h   # 事件控制器
│   ├── fsmonitor.*h           # 监控器主类
│   └── fsmonitorworker.*h     # 工作线程
├── task/                   # 任务处理模块
│   ├── taskmanager.*h          # 任务管理器
│   ├── taskhandler.*h          # 任务处理器
│   ├── indextask.*h            # 索引任务
│   ├── fileprovider.*h         # 文件提供者
│   ├── moveprocessor.*h        # 移动处理器
│   └── progressnotifier.*h     # 进度通知
├── utils/                  # 工具类
│   ├── indexutility.*h         # 索引工具
│   ├── indextraverseutils.*h   # 遍历工具
│   ├── pathexcludematcher.*h   # 路径排除匹配
│   ├── docutils.*h             # 文档处理工具
│   ├── textindexconfig.*h      # 配置管理
│   ├── taskstate.h             # 任务状态
│   └── scopeguard.h            # RAII 守卫
├── private/                # 私有实现
│   └── textindexdbus_p.h       # DBus 私有类
├── textindexdbus.*h       # DBus 服务接口
├── plugin.cpp             # 服务插件入口
└── CMakeLists.txt         # 构建配置
```

---

## 核心组件

### 1. TextIndexDBus (服务入口层)

**职责**: DBus 服务接口，协调 TaskManager 和 FSEventController

**关键文件**:
- `src/services/textindex/textindexdbus.h`
- `src/services/textindex/private/textindexdbus_p.h`
- `src/services/textindex/plugin.cpp`

**DBus 接口**:
- **服务名**: `org.deepin.Filemanager.TextIndex`
- **对象路径**: `/org/deepin/Filemanager/TextIndex`

**主要方法**:
```cpp
void Init();                                    // 初始化并静默启动
bool IsEnabled();                               // 检查监控是否启用
void SetEnabled(bool enabled);                  // 设置启用状态
bool CreateIndexTask(const QStringList &paths);  // 创建索引任务
bool UpdateIndexTask(const QStringList &paths);  // 更新索引任务
bool StopCurrentTask();                         // 停止当前任务
bool HasRunningTask();                          // 检查是否有运行任务
bool IndexDatabaseExists();                     // 检查索引数据库是否存在
QString GetLastUpdateTime();                    // 获取最后更新时间
bool ProcessFileChanges(const QStringList &created,
                       const QStringList &modified,
                       const QStringList &deleted);  // 处理文件变化
bool ProcessFileMoves(const QHash<QString, QString> &movedFiles);  // 处理文件移动
```

**信号**:
```cpp
void TaskFinished(const QString &type, const QString &path, bool success);
void TaskProgressChanged(const QString &type, const QString &path,
                        quint64 count, quint64 total);
```

---

### 2. TaskManager (任务管理层)

**职责**: 管理索引任务的生命周期，维护任务队列

**关键文件**:
- `src/services/textindex/task/taskmanager.h`
- `src/services/textindex/task/taskmanager.cpp`

**核心功能**:
```cpp
// 任务创建和启动
void startTask(const TaskQueueItem &item);
void startFileListTask(const QStringList &fileList, IndexTask::Type type);
void startFileMoveTask(const QHash<QString, QString> &movedFiles);

// 任务队列管理
QQueue<TaskQueueItem> taskQueue;

// 独立工作线程
QThread *workerThread;

// 索引损坏自动重建
void rebuildIndexIfNeeded(bool fatalOccurred);
```

**任务队列调度**:
```
新任务请求 → 检查运行任务 → 有任务则入队 / 无任务则立即启动
    ↓
任务完成 → 处理损坏 → 清理资源 → 启动队列下一个任务
```

---

### 3. IndexTask (索引任务单元)

**职责**: 单个索引任务的执行单元

**关键文件**:
- `src/services/textindex/task/indextask.h`
- `src/services/textindex/task/indextask.cpp`

**任务类型**:
```cpp
enum class Type {
    Create,           // 创建完整索引（清空后重建）
    Update,           // 更新索引（清理+增量）
    CreateFileList,   // 基于文件列表创建
    UpdateFileList,   // 基于文件列表更新
    RemoveFileList,   // 基于文件列表删除
    MoveFileList      // 处理文件移动
};
```

**CPU 节流**:
```cpp
// 静默模式下通过 SystemdCpuUtils 限制 CPU 使用
if (taskInfo.silent) {
    SystemdCpuUtils::limitCpuUsage(cpuUsageLimitPercent);
}
```

**信号**:
```cpp
void started();                           // 任务开始
void finished(bool success, bool fatal);   // 任务完成
void progressChanged(quint64 count, quint64 total);  // 进度更新
```

---

### 4. TaskHandler (任务处理器策略)

**职责**: 定义任务处理函数接口和工厂方法

**关键文件**:
- `src/services/textindex/task/taskhandler.h`
- `src/services/textindex/task/taskhandler.cpp`

**核心处理器**:
```cpp
// 完整索引创建
TaskHandler CreateIndexHandler(const QStringList &paths);

// 索引更新（含清理）
TaskHandler UpdateIndexHandler(const QStringList &paths);

// 文件列表创建/更新
TaskHandler CreateOrUpdateFileListHandler(const QStringList &files,
                                         IndexTask::Type type);

// 文件列表删除
TaskHandler RemoveFileListHandler(const QStringList &files);

// 文件移动处理
TaskHandler MoveFileListHandler(const QHash<QString, QString> &movedFiles);
```

**HandlerResult 返回值**:
```cpp
struct HandlerResult {
    bool success;       // 是否成功
    bool interrupted;   // 是否被中断
    bool useAnything;   // 是否使用了 Anything 服务
    bool fatal;         // 是否致命错误（索引损坏）
};
```

---

### 5. FileProvider (文件提供者策略)

**职责**: 抽象不同的文件获取方式

**关键文件**:
- `src/services/textindex/task/fileprovider.h`
- `src/services/textindex/task/fileprovider.cpp`

**实现类**:
- `FileSystemProvider`: 递归遍历文件系统
- `DirectFileListProvider`: 使用 Anything 服务获取文件列表
- `MixedPathListProvider`: 处理混合路径列表

**工厂方法**:
```cpp
// 自动选择提供者
FileProvider *TaskHandlers::createFileProvider(const QString &path,
                                               const QStringList &pathList);
```

---

### 6. MoveProcessor (文件移动处理)

**职责**: 高效处理文件/目录移动索引更新

**关键文件**:
- `src/services/textindex/task/moveprocessor.h`
- `src/services/textindex/task/moveprocessor.cpp`

**处理器**:
- `FileMoveProcessor`: 处理单个文件移动（检查索引、更新内容）
- `DirectoryMoveProcessor`: 处理目录移动（使用前缀查询批量更新）

---

### 7. FSMonitor (文件系统监控)

**职责**: 递归监控文件系统，过滤和保护性监控

**关键文件**:
- `src/services/textindex/fsmonitor/fsmonitor.h`
- `src/services/textindex/fsmonitor/fsmonitor_p.h`
- `src/services/textindex/fsmonitor/fsmonitor.cpp`

**核心功能**:
- 基于 DFileSystemWatcher 的递归监控
- 黑名单、外部挂载、符号链接自动过滤
- inotify watches 资源限制管理
- 可选的快速目录扫描（FastScan）
- 隐藏文件/目录支持

**信号**:
```cpp
void fileCreated(const QString &path);
void fileDeleted(const QString &path);
void fileModified(const QString &path);
void fileMoved(const QString &oldPath, const QString &newPath);
```

---

### 8. FSEventCollector (事件收集器)

**职责**: 批量收集和聚合文件事件

**关键文件**:
- `src/services/textindex/fsmonitor/fseventcollector.h`
- `src/services/textindex/fsmonitor/fseventcollector.cpp`

**核心功能**:
```cpp
// 可配置的事件收集间隔（默认180秒）
int autoIndexUpdateInterval;

// 智能事件合并（如创建+删除抵消）
// 递归添加目录监控，受系统 inotify 限制约束

// 分类收集
QStringList m_collectedCreatedFiles;
QStringList m_collectedDeletedFiles;
QStringList m_collectedModifiedFiles;
QHash<QString, QString> m_collectedMovedFiles;
```

**信号**:
```cpp
void filesCreated(const QStringList &files);
void filesDeleted(const QStringList &files);
void filesModified(const QStringList &files);
void filesMoved(const QHash<QString, QString> &movedFiles);
void flushFinished();
```

---

### 9. FSEventController (事件控制器)

**职责**: 控制文件监控启停和事件处理流程

**关键文件**:
- `src/services/textindex/fsmonitor/fseventcontroller.h`
- `src/services/textindex/fsmonitor/fseventcontroller.cpp`

**核心功能**:
- 延迟启动监控（可配置延迟）
- 静默启动管理（首次启动后台更新）
- 监控启停控制（延迟清理资源）
- 事件处理和任务调度

---

## 工具类

### IndexUtility

**职责**: 索引相关实用工具和状态管理

**关键文件**: `src/services/textindex/utils/indexutility.h`

**核心功能**:
- 索引状态管理（Clean/Dirty/Unknown）
- 索引状态文件读写
- 版本兼容性检查
- 文件支持性检查
- Anything 配置监听

---

### PathExcludeMatcher

**职责**: 路径排除规则匹配

**关键文件**: `src/services/textindex/utils/pathexcludematcher.h`

**匹配模式**:
- `ExactName`: 精确目录名匹配（如 "tmp", ".git"）
- `PathSegment`: 路径段匹配（如 ".local/share/Trash"）
- `AbsolutePrefix`: 绝对路径前缀匹配
- `GlobPattern`: Glob 通配符匹配（如 "build-*", "*.cache"）

**工厂方法**:
```cpp
PathExcludeMatcher *PathExcludeMatcher::createForIndex();
```

---

### DocUtils

**职责**: 文档内容提取和处理

**关键文件**: `src/services/textindex/utils/docutils.h`

**核心功能**:
- 文件编码检测
- 文本编码转换
- 文件内容提取（含 HTML 处理）
- Lucene 文档操作辅助

---

### TextIndexConfig

**职责**: 配置管理和热重载

**关键文件**: `src/services/textindex/utils/textindexconfig.h`

**配置项**:
```cpp
autoIndexUpdateInterval          // 自动索引更新间隔（秒）
monitoringStartDelaySeconds      // 监控启动延迟（秒）
silentIndexUpdateDelay           // 静默更新延迟（秒）
maxIndexFileSizeMB               // 最大索引文件大小（MB）
maxIndexFileTruncationSizeMB     // 文件截断大小（MB）
supportedFileExtensions          // 支持的文件扩展名
indexHiddenFiles                 // 是否索引隐藏文件
folderExcludeFilters             // 文件夹排除过滤器
cpuUsageLimitPercent             // CPU 使用限制百分比
inotifyWatchesCoefficient        // inotify 系数
```

---

## 工作流程

### 1. 服务启动流程

```
DSMRegister() [plugin.cpp]
    ↓
创建 TextIndexDBus 实例
    ↓
降低进程优先级
    ↓
TextIndexDBusPrivate::initialize()
    ├─ setupFSEventCollector()
    │  └─ 创建 FSEventCollector
    ├─ initializeSupportedExtensions()
    │  └─ 从配置加载支持的扩展名
    └─ initConnect()
       ├─ 连接 TaskManager 信号
       ├─ 连接 FSEventController 信号
       └─ 连接 TextIndexConfig 信号
```

---

### 2. 索引创建/更新流程

**手动触发**:
```
CreateIndexTask/UpdateIndexTask (DBus 调用)
    ↓
TextIndexDBus::CreateIndexTask()/UpdateIndexTask()
    ↓
TaskManager::startTask()
    ├─ 验证路径有效性（必须是默认索引目录）
    ├─ 如果有运行任务 → 停止并加入队列
    ├─ 创建 IndexTask 和对应的 TaskHandler
    ├─ 将任务移至 workerThread
    ├─ 标记索引状态为 Dirty
    └─ 发送 startTaskInThread 信号启动任务
         ↓
    IndexTask::doTask()
    ├─ 应用 CPU 节流（静默模式）
    ├─ 调用 TaskHandler
    │  ├─ 获取 FileProvider
    │  │  ├─ 尝试使用 Anything 服务
    │  │  └─ 或使用 FileSystemProvider
    │  ├─ 遍历文件
    │  │  ├─ 过滤扩展名、黑名单等
    │  │  └─ 调用 processFile/updateFile
    │  │     ├─ 调用 DocUtils 提取内容
    │  │     ├─ 创建/更新 Lucene Document
    │  │     └─ 批量提交（可配置间隔）
    │  ├─ 提交更改
    │  └─ 优化索引
    ├─ 处理异常（标记损坏）
    └─ 发送 finished 信号
         ↓
    TaskManager::onTaskFinished()
    ├─ 更新索引状态文件
    ├─ 如果索引损坏 → 重建索引
    └─ 启动下一个队列任务
```

---

### 3. 文件监控和增量索引流程

```
FSMonitor 检测文件事件（通过 inotify）
    ↓
FSMonitor 发送信号：fileCreated/fileDeleted/fileModified/fileMoved
    ↓
FSEventCollector 收集事件（批量）
    ├─ 创建 + 删除 → 抵消
    ├─ 路径移动 → 合并为 movedFiles
    └─ 根据收集间隔 Flush
         ↓
    FSEventCollector 发送批量信号
    └─ filesCreated/filesDeleted/filesModified/filesMoved
         ↓
    FSEventController 收集批次事件
    └─ 清除批次缓存
         ↓
    发送 requestProcessFileChanges/ProcessFileMoves
         ↓
    TextIndexDBus 内部调用
    ├─ deletedFiles → RemoveFileList 任务
    ├─ createdFiles → CreateFileList 任务
    └─ modifiedFiles → UpdateFileList 任务
         ↓
    TaskManager 启动文件列表任务
    └─ 在队列中处理或立即执行
```

---

## 关键数据结构

### 任务队列项

```cpp
struct TaskQueueItem {
    IndexTask::Type type;                    // 任务类型
    QString path;                            // 主路径（用于日志）
    QStringList pathList;                    // 多路径列表
    QStringList fileList;                    // 文件列表（文件列表任务）
    QHash<QString, QString> movedFiles;      // 移动映射 (oldPath -> newPath)
    bool silent;                             // 静默模式
};
```

---

### 任务处理器结果

```cpp
struct HandlerResult {
    bool success;       // 是否成功
    bool interrupted;   // 是否被中断
    bool useAnything;   // 是否使用了 Anything 服务
    bool fatal;         // 是否致命错误（索引损坏）
};
```

---

### Lucene Document 字段

| 字段名 | 存储 | 索引 | 分析 | 说明 |
|-------|------|------|------|------|
| `path` | 是 | 否 | 否 | 文件完整路径 |
| `ancestor_paths` | 否 | 否 | 否 | 祖先路径列表（用于目录查询优化） |
| `modified` | 是 | 否 | 否 | 最后修改时间 |
| `filename` | 是 | 是 | 是 | 文件名 |
| `is_hidden` | 是 | 否 | 否 | 是否隐藏 |
| `contents` | 是 | 是 | 是 | 文件内容 |

---

## 配置文件

### DBus 接口定义

**位置**: `assets/dbus/org.deepin.Filemanager.TextIndex.xml`

### Systemd 限制

**内存限制**: `systemd/memory-limit.conf`

---

## 最佳实践

### Do
- 使用 FileProvider 策略模式扩展文件获取方式
- 使用 TaskHandler 实现不同的索引策略
- 使用 FSEventCollector 批量处理事件
- 使用 CPU 节流在静默模式下降低资源消耗
- 使用任务队列避免任务冲突

### Don't
- 不要在监控路径中包含外部挂载点
- 不要忽略索引损坏的重建流程
- 不要直接修改索引状态文件（使用 IndexUtility）
- 不要跳过路径排除过滤器

---

## 参考文档

- [数据库指南](database-guidelines.md) - SQLite ORM 使用
- [错误处理指南](error-handling.md) - FinallyUtil 使用
- [日志指南](logging-guidelines.md) - 分类日志使用
