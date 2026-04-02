# dde-file-manager-extractor 进程隔离架构

## Goal

实现一个进程隔离的文件内容提取器，将不稳定的文件提取操作（文本、未来OCR、多媒体特征等）隔离到独立进程，保护 textindex 服务稳定性。

## Requirements

### 功能需求

1. **进程隔离**
   - extractor 作为独立进程运行
   - 主进程通过 stdin/stdout 管道与 extractor 通信
   - extractor 崩溃不影响主进程

2. **IPC 通信**
   - 基于 QDataStream 的二进制序列化
   - 支持 10MB+ 大文本传输
   - 使用事务机制确保消息完整性
   - 消息类型：Status（状态）+ Data（数据）

3. **插件架构**
   - 支持动态加载提取器插件
   - 插件运行时探测能否处理给定文件（不被 mimetype 限制）
   - 首期实现：文本文档内容提取插件

4. **生命周期管理**
   - 批量模式：textindex 启动 → 发送任务 → 等待完成 → 关闭
   - 支持批处理多个文件

5. **共享库**
   - 创建 libextractor.so 封装 IPC 通信能力
   - 方便测试程序和其他模块复用

### 非功能需求

1. **不依赖 KDE 组件**
2. **依赖 dfm-base**（如 ProcessPriorityManager）
3. **遵循 SOLID、DRY、KISS 原则**
4. **C++17、Qt6、CMake 3.10+**
5. **编码规范**：禁止相对路径 include

## Acceptance Criteria

- [ ] extractor 可执行文件能独立运行
- [ ] libextractor.so 提供 ControllerPipe 和 WorkerPipe 类
- [ ] 文本提取插件正确实现并加载
- [ ] 测试程序能模拟主进程与 extractor 完整交互
- [ ] 支持提取 10MB+ 文本内容
- [ ] extractor 异常退出时主进程能正确处理

## Technical Notes

### 目录结构

```
src/apps/dde-file-manager-extractor/
├── CMakeLists.txt
├── extractor.json              # 主程序元数据
├── libextractor/               # 共享库
│   ├── CMakeLists.txt
│   ├── controllerpipe.h/cpp    # 主进程端通信
│   ├── workerpipe.h/cpp        # 子进程端通信
│   ├── extractortypes.h        # 类型定义
│   └── extractor_global.h
├── extractor/                  # 子进程主程序
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── extractorapp.h/cpp      # 子进程逻辑
│   └── pluginloader.h/cpp      # 插件加载
├── plugins/                    # 插件目录
│   ├── CMakeLists.txt
│   ├── extractor_plugin_global.h
│   └── text-extractor/         # 文本提取插件
│       ├── CMakeLists.txt
│       ├── textextractor.json
│       ├── textextractorplugin.h/cpp
│       └── textextractor.h/cpp
└── tests/                      # 测试程序（独立项目）
    └── extractor-test/         # 已存在的测试目录扩展
```

### IPC 协议设计

```
主进程 -> 子进程 (命令流):
  QVector<QString> filePaths    // 批量文件路径

子进程 -> 主进程 (状态流):
  quint8 status                 // 状态码
  QString path                  // 当前处理的文件路径
  QByteArray data               // 提取内容（仅 Data 状态）

状态码:
  Started     = 'S'    // 开始处理某文件
  Finished    = 'F'    // 成功完成
  Failed      = 'f'    // 处理失败
  Data        = 'D'    // 数据就绪 (path + data)
  BatchDone   = 'B'    // 批次完成
```

### 插件接口设计

```cpp
class AbstractExtractorPlugin : public QObject {
    Q_OBJECT
public:
    // 探测能否处理该文件
    virtual bool canExtract(const QString &filePath) const = 0;

    // 执行提取，返回提取内容
    virtual std::optional<QByteArray> extract(const QString &filePath) = 0;

    // 插件名称
    virtual QString name() const = 0;
};
```

### 参考

- preview 插件架构：src/apps/dde-file-manager-preview/
- textindex 服务：src/services/textindex/
