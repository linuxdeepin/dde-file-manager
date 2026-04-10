# TextIndex 服务技术规范

> DDE File Manager 文本索引服务 - 基于 Lucene++ 的全文检索后台服务，支持文本内容和 OCR 图像识别双索引。

---

## 概述

TextIndex 服务是 DDE File Manager 的独立文件文本索引服务，提供基于 Lucene++ 的全文检索能力。服务采用分层架构设计，通过 DBus 提供跨进程接口，支持实时文件系统监控和增量索引更新。

**核心能力**:
- **双索引模式**: 文本内容索引 + OCR 图像文字识别索引
- **进程隔离**: 提取器独立子进程运行，崩溃不影响主服务
- **插件化提取**: 支持动态加载文本/OCR 提取插件
- **智能事件聚合**: 批量处理文件系统变更事件
- **资源控制**: CPU 节流、空闲超时、inotify 限制管理
- **静默后台更新**: 可配置延迟启动和资源占用限制

---

## 技术栈

| 依赖 | 版本/类型 | 用途 |
|------|-----------|------|
| Qt6 | Core, DBus, Gui | 基础框架 |
| Dtk6::Core | DFileSystemWatcher, DConfig | 文件系统监控、配置管理 |
| dfm6-search | - | 全文检索基础 |
| Lucene++ | liblucene++, liblucene++-contrib | 全文检索引擎 |
| dde-file-manager-extractor | 子进程工具 | 文本/OCR 提取 |

---

## 架构设计

### 整体架构图

```
+------------------------------------------------------------------+
|                        DBus Interface Layer                       |
|  +-------------------------+  +-----------------------------+    |
|  | TextIndexDBus           |  | OcrIndexDBus                |    |
|  | org.deepin.Filemanager. |  | org.deepin.Filemanager.     |    |
|  | TextIndex               |  | OcrIndex                    |    |
|  +-------------------------+  +-----------------------------+    |
+------------------------------------------------------------------+
                                |
                                v
+------------------------------------------------------------------+
|                     Runtime Layer (IndexRuntime)                  |
|  +------------+  +---------------+  +-------------+  +---------+ |
|  | IndexProfile| | IndexStateStore| | TaskManager | | FSEvent | |
|  |             |  |               |  |             | |Controller| |
|  +------------+  +---------------+  +-------------+  +---------+ |
+------------------------------------------------------------------+
        |                   |                  |               |
        v                   v                  v               v
+--------------+  +------------------+  +------------+  +------------+
| IndexContext |  | ProcessExtractor |  | IndexTask  |  | FSEvent    |
|              |  |      +           |  |            |  | Collector  |
| Profile      |  | ProcessExtractor |  | TaskHandler|  |            |
| StateStore   |  |     Proxy        |  | FileProvider|  | FSMonitor  |
| Extractor    |  |                  |  | MoveProcessor| |            |
| DocBuilder   |  +------------------+  +------------+  +------------+
+--------------+           |                                  |
        |                  v                                  v
        v       +-------------------+              +------------------+
+--------------+ |  dde-file-manager |              | DFileSystem     |
| Document     | |  -extractor       |              | Watcher         |
| Builder      | | (subprocess)      |              | (inotify)       |
| +            | +-------------------+              +------------------+
| ControllerPipe  |
| WorkerPipe       |
+--------------+
```

---

## 目录结构

```
textindex/
├── core/                    # 核心运行时
│   ├── indexcontext.h           # 索引上下文 (Lucene 读写器管理)
│   └── indexruntime.h           # 运行时容器 (Profile/Extractor/Builder)
├── fsmonitor/               # 文件系统监控模块
│   ├── fseventcollector.*h      # 事件收集器 (批量聚合)
│   ├── fseventcontroller.*h     # 事件控制器 (启停管理)
│   ├── fsmonitor.*h             # 监控器主类 (inotify 封装)
│   └── fsmonitorworker.*h       # 工作线程
├── task/                    # 任务处理模块
│   ├── taskmanager.*h           # 任务管理器 (队列调度)
│   ├── taskhandler.*h           # 任务处理器 (策略模式)
│   ├── indextask.*h             # 索引任务 (工作线程)
│   ├── fileprovider.*h          # 文件提供者 (遍历策略)
│   ├── moveprocessor.*h         # 移动处理器 (路径更新)
│   └── progressnotifier.*h      # 进度通知
├── extractor/               # 提取器模块
│   ├── indexextractor.h         # 提取器接口
│   └── processextractor.*h      # 进程提取器 (子进程 IPC)
├── document/                # 文档构建模块
│   ├── indexdocumentbuilder.h   # 文档构建器接口
│   ├── contentdocumentbuilder.*h # 文本内容构建器
│   └── ocrdocumentbuilder.*h    # OCR 文档构建器
├── profile/                 # 索引配置
│   └── indexprofile.*h          # 索引类型定义
├── state/                   # 状态管理
│   └── indexstatestore.*h       # 状态持久化
├── utils/                   # 工具类
│   ├── indexutility.*h          # 索引工具
│   ├── indextraverseutils.*h    # 遍历工具
│   ├── pathexcludematcher.*h    # 路径排除匹配
│   ├── docutils.*h              # Lucene 文档操作辅助
│   ├── textindexconfig.*h       # 配置管理 (DConfig)
│   ├── taskstate.h              # 任务状态枚举
│   ├── scopeguard.h             # RAII 守卫
│   └── systemdcpuutils.*h       # CPU 节流工具
├── private/                 # 私有实现
│   ├── textindexdbus_p.h        # TextIndex DBus 私有类
│   └── ocrindexdbus_p.h         # OcrIndex DBus 私有类
├── textindexdbus.*h         # 文本索引 DBus 服务接口
├── ocrindexdbus.*h          # OCR 索引 DBus 服务接口
├── plugin.cpp               # 服务插件入口
└── CMakeLists.txt           # 构建配置
```

---

## 核心组件

### 1. 服务入口 (Plugin)

**文件**: `src/services/textindex/plugin.cpp`

**职责**: 注册两个 DBus 服务

```cpp
// 注册服务
QObject *DSMRegister(QObject *parent, const QStringList &args)
{
    // TextIndexDBus: 文本内容索引
    // OcrIndexDBus: OCR 图像索引
}
```

---

### 2. IndexRuntime (运行时容器)

**职责**: 管理索引运行时组件的生命周期

**文件**: `src/services/textindex/core/indexruntime.h`

```cpp
class IndexRuntime : public QObject
{
public:
    IndexRuntime(IndexProfile profile, QObject *parent = nullptr);
    
    // 选择提取器和文档构建器
    const IndexExtractor *selectExtractor() const;
    const IndexDocumentBuilder *selectDocumentBuilder() const;
    
private:
    IndexProfile m_profile;           // 索引类型配置
    IndexStateStore m_stateStore;     // 状态存储
    ProcessExtractor m_processExtractor;  // 统一提取器
    ContentDocumentBuilder m_contentDocumentBuilder;
    OcrDocumentBuilder m_ocrDocumentBuilder;
};
```

**设计要点**:
- 统一使用 `ProcessExtractor` 处理所有提取请求
- 根据 `IndexProfile` 选择对应的文档构建器

---

### 3. ProcessExtractor (进程提取器)

**职责**: 通过子进程隔离执行文本/OCR 提取

**文件**: `src/services/textindex/extractor/processextractor.cpp`

**架构设计**:

```
+------------------+     IPC 协议     +-------------------+
| ProcessExtractor | <=============> | dde-file-manager  |
|       +          |                 | -extractor        |
| ProcessExtractor |                 | (subprocess)      |
|     Proxy        |                 +-------------------+
+------------------+                         |
        |                                    v
        |                           +------------------+
        |                           | Extractor Plugin |
        |                           | - text-extractor |
        |                           | - ocr-extractor  |
        |                           +------------------+
        v
  ControllerPipe
```

**ProcessExtractorProxy 关键特性**:

| 特性 | 配置 | 说明 |
|------|------|------|
| 懒加载 | - | 首次请求时启动子进程 |
| 请求超时 | 120s | 未响应自动终止 |
| 空闲超时 | 60s | 无活动自动关闭子进程 |
| 崩溃恢复 | 自动 | 检测崩溃并重启 |
| 线程安全 | BlockingQueuedConnection | 支持跨线程调用 |

**关键常量**:
```cpp
constexpr int kExtractorRequestTimeoutMs = 120000;  // 请求超时
constexpr int kExtractorIdleShutdownMs = 60000;     // 空闲关闭
```

**使用示例**:
```cpp
ProcessExtractor extractor;
IndexExtractionResult result = extractor.extract(filePath, maxBytes);
if (result.success) {
    // result.text 包含提取内容
}
```

---

### 4. ControllerPipe IPC 协议

**文件**: `src/apps/dde-file-manager-extractor/libextractor/controllerpipe.h`

**协议格式**:
```
+----------------+------------------+
| Size (qint32)  | Payload (QByteArray) |
+----------------+------------------+
```

**状态码 (ExtractorStatus)**:

| 状态 | 值 | 方向 | 说明 |
|------|-----|------|------|
| `Started` | 'S' | Worker→Controller | 开始处理文件 |
| `Finished` | 'F' | Worker→Controller | 成功完成 |
| `Failed` | 'f' | Worker→Controller | 处理失败 |
| `Data` | 'D' | Worker→Controller | 数据就绪 |
| `BatchDone` | 'B' | Worker→Controller | 批量完成 |

**请求流程**:
```
Controller.extractBatch([file1, file2, ...])
    --> Worker: Started (file1)
    --> Worker: Data (file1, content)
    --> Worker: Started (file2)
    --> Worker: Data (file2, content)
    --> Worker: BatchDone
```

---

### 5. dde-file-manager-extractor (提取工具)

**目录**: `src/apps/dde-file-manager-extractor/`

```
dde-file-manager-extractor/
├── extractor/
│   ├── main.cpp           # 入口 (命令行/服务模式)
│   ├── extractorapp.h     # 主应用类
│   └── pluginloader.cpp   # 插件加载器
├── libextractor/
│   ├── controllerpipe.h   # 控制端管道
│   ├── workerpipe.h       # 工作端管道
│   └── extractortypes.h   # 类型定义
└── plugins/
    ├── text-extractor/    # 文本提取插件
    └── ocr-extractor/     # OCR 提取插件
```

**插件接口**:
```cpp
class ExtractorPlugin
{
public:
    virtual ~ExtractorPlugin() = default;
    virtual QStringList supportedExtensions() const = 0;
    virtual std::optional<QString> extract(const QString &filePath, size_t maxBytes) = 0;
};
```

**命令行模式**:
```bash
# 单文件提取
dde-file-manager-extractor /path/to/file.txt

# 批量目录处理
dde-file-manager-extractor --batch /path/to/directory
```

---

### 6. TaskManager (任务管理器)

**职责**: 管理索引任务的生命周期，维护任务队列

**文件**: `src/services/textindex/task/taskmanager.h`

**任务类型**:
```cpp
enum class Type {
    Create,           // 创建完整索引
    Update,           // 更新索引 (清理+增量)
    CreateFileList,   // 基于文件列表创建
    UpdateFileList,   // 基于文件列表更新
    RemoveFileList,   // 基于文件列表删除
    MoveFileList      // 处理文件移动
};
```

**任务调度流程**:
```
startTask() --> 验证路径 --> 检查运行任务
                           |-- 有任务 --> 入队等待
                           |-- 无任务 --> 立即启动
                                   |
                                   v
                           IndexTask (工作线程)
                                   |
                                   v
                           onTaskFinished() --> 检查队列 --> 启动下一个
```

**CPU 节流**:
```cpp
// 静默模式下通过 systemd 限制 CPU
if (taskInfo.silent) {
    SystemdCpuUtils::limitCpuUsage(cpuUsageLimitPercent);
}
```

---

### 7. FSMonitor 三层架构

#### 7.1 FSMonitor (监控器)

**职责**: 基于 inotify 的递归目录监控

**文件**: `src/services/textindex/fsmonitor/fsmonitor.h`

**特性**:
- 递归监控目录树
- 资源限制管理 (不超过系统 inotify watch 限制的百分比)
- 路径排除 (黑名单、符号链接、网络挂载、隐藏文件)

**信号**:
```cpp
Q_SIGNALS:
    void fileCreated(const QString &path);
    void fileDeleted(const QString &path);
    void fileModified(const QString &path);
    void fileMoved(const QString &oldPath, const QString &newPath);
```

#### 7.2 FSEventCollector (事件收集器)

**职责**: 批量聚合文件系统事件

**文件**: `src/services/textindex/fsmonitor/fseventcollector.h`

**智能事件合并**:
| 事件序列 | 合并结果 |
|----------|----------|
| 创建 + 删除 | 抵消 (无操作) |
| 创建 + 修改 | 仅创建 |
| 移动 | 路径更新 (而非删除+创建) |

**配置**:
```cpp
autoIndexUpdateInterval = 3;  // 收集间隔 (秒)
maxCollectedEvents = 10000;   // 最大事件数限制
```

#### 7.3 FSEventController (事件控制器)

**职责**: 控制监控启停和事件分发

**文件**: `src/services/textindex/fsmonitor/fseventcontroller.h`

**延迟配置**:
| 配置 | 默认值 | 说明 |
|------|--------|------|
| monitoringStartDelaySeconds | 30s | 监控启动延迟 |
| silentIndexUpdateDelay | 180s | 静默启动延迟 |
| inotifyResourceCleanupDelay | 30min | 资源清理延迟 |

---

### 8. 文档构建器

#### 8.1 ContentDocumentBuilder

**职责**: 构建文本内容索引文档

**Lucene 字段定义**:

| 字段 | 存储 | 索引 | 分析 | 说明 |
|------|------|------|------|------|
| `path` | YES | NOT_ANALYZED | - | 文件完整路径 |
| `ancestor_paths` | NO | NOT_ANALYZED | - | 祖先目录列表 |
| `modified` | YES | NOT_ANALYZED | - | 修改时间戳 |
| `filename` | YES | ANALYZED | YES | 文件名 (分词) |
| `is_hidden` | YES | NOT_ANALYZED | - | 隐藏标记 |
| `contents` | YES | ANALYZED | YES | 文本内容 (分词) |

#### 8.2 OcrDocumentBuilder

**职责**: 构建 OCR 图像索引文档

**支持的图片格式**:
```cpp
const QStringList defaultSupportedOcrImageExtensions = {
    "ani", "bmp", "jpe", "jpeg", "jpg", "pcx", "png", "psd",
    "tga", "tif", "tiff", "webp", "wmf", "heic", "heif", "raw"
};
```

**配置**:
- `maxOcrImageSizeMB`: 30 (OCR 图片最大尺寸)

---

### 9. TextIndexConfig (配置管理)

**职责**: 通过 DConfig 管理运行时配置

**文件**: `src/services/textindex/utils/textindexconfig.h`

**配置项列表**:

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `autoIndexUpdateInterval` | 3 | 事件收集间隔 (秒) |
| `monitoringStartDelaySeconds` | 30 | 监控启动延迟 (秒) |
| `silentIndexUpdateDelay` | 180 | 静默启动延迟 (秒) |
| `inotifyResourceCleanupDelay` | 1800000 | 资源清理延迟 (毫秒) |
| `maxIndexFileSizeMB` | 50 | 最大索引文件大小 (MB) |
| `maxIndexFileTruncationSizeMB` | 10 | 截断大小 (MB) |
| `supportedFileExtensions` | [列表] | 支持的文件扩展名 |
| `supportedOcrImageExtensions` | [列表] | 支持的图片格式 |
| `maxOcrImageSizeMB` | 30 | OCR 图片最大大小 (MB) |
| `indexHiddenFiles` | false | 是否索引隐藏文件 |
| `folderExcludeFilters` | [列表] | 排除目录列表 |
| `cpuUsageLimitPercent` | 50 | CPU 使用限制 (%) |
| `inotifyWatchesCoefficient` | 0.5 | inotify watch 使用系数 |
| `batchCommitInterval` | 1000 | 批量提交间隔 |

**动态更新**:
```cpp
connect(&TextIndexConfig::instance(), &TextIndexConfig::configChanged,
        this, &FSEventController::onConfigChanged);
```

---

## 工作流程

### 1. 服务启动流程

```
DSMRegister() [plugin.cpp]
    |
    +-- 创建 TextIndexDBus / OcrIndexDBus
    |
    +-- 降低进程优先级
    |
    +-- TextIndexDBusPrivate::initialize()
           |
           +-- setupFSEventCollector()
           |
           +-- initializeSupportedExtensions()
           |
           +-- initConnect()
                  |
                  +-- 连接 TaskManager 信号
                  +-- 连接 FSEventController 信号
                  +-- 连接 TextIndexConfig 信号
```

### 2. 索引创建流程

```
CreateIndexTask(paths) [DBus 调用]
    |
    v
TaskManager::startTask(Create, paths)
    |
    +-- 验证路径有效性
    |
    +-- 创建 IndexTask (移至工作线程)
    |
    +-- 标记索引状态 Dirty
    |
    v
IndexTask::doTask() [工作线程]
    |
    +-- 应用 CPU 节流 (静默模式)
    |
    +-- CreateIndexHandler
    |      |
    |      +-- FileProvider 遍历文件
    |      |      |
    |      |      +-- 过滤扩展名/黑名单
    |      |      |
    |      |      v
    |      |      ProcessExtractor::extract()
    |      |             |
    |      |             +-- ProcessExtractorProxy
    |      |             |      |
    |      |             |      +-- 启动/复用子进程
    |      |             |      |
    |      |             |      +-- ControllerPipe IPC
    |      |             |             |
    |      |             |             v
    |      |             |      dde-file-manager-extractor
    |      |             |             |
    |      |             |             +-- text-extractor / ocr-extractor
    |      |             |
    |      |             v
    |      |      IndexExtractionResult
    |      |
    |      +-- ContentDocumentBuilder / OcrDocumentBuilder
    |      |
    |      +-- IndexWriter::addDocument()
    |      |
    |      +-- 批量提交 (每 1000 文件)
    |
    +-- IndexWriter::optimize()
    |
    v
TaskManager::onTaskFinished()
    |
    +-- 更新索引状态 Clean
    |
    +-- 启动队列下一个任务
```

### 3. 增量更新流程

```
FSMonitor 检测文件事件 (inotify)
    |
    v
FSEventCollector 收集事件 (批量)
    |   |-- 创建 + 删除 → 抵消
    |   |-- 路径移动 → 合并
    |   +-- 收集间隔到期
    |
    v
FSEventController 分发事件
    |
    v
TextIndexDBus::ProcessFileChanges / ProcessFileMoves
    |
    +-- deletedFiles --> RemoveFileList 任务
    +-- createdFiles --> CreateFileList 任务
    +-- modifiedFiles --> UpdateFileList 任务
    +-- movedFiles --> MoveFileList 任务
    |
    v
TaskManager 调度执行
```

---

## DBus 接口

### TextIndexDBus

**服务名**: `org.deepin.Filemanager.TextIndex`  
**对象路径**: `/org/deepin/Filemanager/TextIndex`

**方法**:

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Init` | - | void | 初始化服务 |
| `IsEnabled` | - | bool | 检查索引是否启用 |
| `SetEnabled` | bool | void | 启用/禁用索引 |
| `CreateIndexTask` | QStringList | bool | 创建索引任务 |
| `UpdateIndexTask` | QStringList | bool | 更新索引任务 |
| `StopCurrentTask` | - | bool | 停止当前任务 |
| `HasRunningTask` | - | bool | 检查是否有运行任务 |
| `IndexDatabaseExists` | - | bool | 检查索引是否存在 |
| `GetLastUpdateTime` | - | QString | 获取最后更新时间 |
| `ProcessFileChanges` | created, modified, deleted | bool | 处理文件变更 |
| `ProcessFileMoves` | QHash<QString, QString> | bool | 处理文件移动 |

**信号**:

| 信号 | 参数 | 说明 |
|------|------|------|
| `TaskFinished` | type, path, success | 任务完成 |
| `TaskProgressChanged` | type, path, count, total | 进度更新 |

### OcrIndexDBus

**服务名**: `org.deepin.Filemanager.OcrIndex`  
**对象路径**: `/org/deepin/Filemanager/OcrIndex`

接口与 TextIndexDBus 相同，用于 OCR 图像索引。

---

## 索引版本管理

| 索引类型 | 版本常量 | 说明 |
|----------|----------|------|
| 文本内容 | `kTextIndexVersion = 2` | 文本内容索引版本 |
| OCR 图像 | `kOcrIndexVersion = 0` | OCR 索引版本 |

**版本兼容性**: 索引版本变更时自动重建索引。

---

## 最佳实践

### Do
- 使用 `ProcessExtractor` 进行所有文本提取 (进程隔离)
- 使用 `TaskManager` 任务队列避免并发冲突
- 使用 `FSEventCollector` 批量处理事件
- 使用 `TextIndexConfig` DConfig 进行动态配置
- 使用 CPU 节流控制后台资源占用

### Don't
- 不要跳过路径排除过滤器
- 不要忽略索引状态 (Clean/Dirty)
- 不要在主线程执行提取操作
- 不要绕过任务队列直接操作索引

---

## 故障恢复

### 提取器崩溃
1. `ProcessExtractorProxy` 检测 `processCrashed` 信号
2. 终止当前请求，返回错误
3. 下次请求自动重启子进程

### 索引损坏
1. `TaskHandler` 捕获 Lucene 异常
2. 标记索引状态为 Corrupted
3. 触发自动重建流程

---

## 参考文档

- [数据库指南](database-guidelines.md) - SQLite ORM 使用
- [错误处理指南](error-handling.md) - FinallyUtil 使用
- [日志指南](logging-guidelines.md) - 分类日志使用
- [配置管理](qt-dtk-guide.md) - DConfig 使用
