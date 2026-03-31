# Bug 修复规范

> DDE 应用 Bug 修复的规范和约束。

---

## 概览

本指南定义 Bug 修复的标准流程和约束，遵循最小修改原则。

---

## 核心原则

### 最小修改原则

Bug 修复必须遵循最小修改范围：
- 只修复问题本身，不重构周边代码
- 不添加新功能
- 保持现有代码风格
- 添加注释说明修复原因

```cpp
// ❌ 错误: 重构整段代码
void processFile() {
    // 完全重写的代码...
}

// ✅ 正确: 只修复 Bug
void processFile() {
    // ... 原有代码 ...
    if (file.isOpen()) {  // 添加缺失的检查
        file.write(data);
    }
    // ... 原有代码 ...
}
```

### 根因分析优先

在修复之前必须找到根本原因。使用 5-Why 方法系统性分析。

---


## 修复流程

### 1. 信息收集

- 收集 Bug 描述、复现步骤、期望结果
- 获取日志、截图和环境信息
- 确认影响范围和优先级

### 2. 根因分析

使用 `guides/root-cause-analysis.md` 中的 5-Why 方法：

| Level | Question | Answer | Evidence |
|-------|----------|--------|----------|
| Why 1 | 为什么会出现这个现象？ | [直接原因] | [证据] |
| Why 2 | 为什么会有这个直接原因？ | [深层原因] | [证据] |
| Why 3 | 为什么允许这个条件？ | [系统原因] | [证据] |
| Why 4 | 为什么设计允许这个问题？ | [设计原因] | [证据] |
| Why 5 | 根本原因是什么？ | [根本原因] | [证据] |

### 3. 设计修复方案

- 针对根因设计最小修复
- 评估修复的影响范围
- 用户确认方案

### 4. 实施修改

- 在最小范围内修改代码
- 添加注释说明修复原因
- 确保不引入新问题

### 5. 验证

- 编译通过，无警告
- 运行相关测试
- 手动验证修复效果

---

## 修复合约

### 必须做

- [ ] 找到根本原因（5-Why 分析）
- [ ] 设计最小修复方案
- [ ] 添加修复注释
- [ ] 验证编译和测试
- [ ] 手动验证修复效果

### 禁止做

- [ ] 重构周边代码
- [ ] 添加新功能
- [ ] 修改不相关的逻辑
- [ ] 改变现有接口签名
- [ ] 跳过根因分析

---

## 常见修复模式

### 边界条件修复

```cpp
// ❌ 错误: 没有检查边界
QString getFileContent(const QString &path) {
    QFile file(path);
    return file.readAll();  // 如果文件不存在会失败
}

// ✅ 正确: 添加边界检查
QString getFileContent(const QString &path) {
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "File not found:" << path;
        return QString();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << path;
        return QString();
    }
    return file.readAll();
}
```

### 空指针修复

```cpp
// ❌ 错误: 可能空指针
void processItem(Item *item) {
    qDebug() << item->name();  // 如果 item 为空会崩溃
}

// ✅ 正确: 添加空指针检查
void processItem(Item *item) {
    if (!item) {
        qWarning() << "Item is null";
        return;
    }
    qDebug() << item->name();
}
```

### 信号槽连接修复

```cpp
// ❌ 错误: 跨线程没有指定连接类型
connect(worker, &Worker::signal, uiWidget, &Widget::slot);

// ✅ 正确: 使用 QueuedConnection
connect(worker, &Worker::signal, uiWidget, &Widget::slot,
        Qt::QueuedConnection);
```

### 内存泄漏修复

```cpp
// ❌ 错误: 没有父对象
DWidget *widget = new DWidget();  // 泄漏

// ✅ 正确: 设置父对象
DWidget *widget = new DWidget(parentWidget);
```

---

## 注释规范

修复必须添加注释说明原因：

```cpp
// Fix: 添加文件存在性检查，防止空指针崩溃
// Bug: #12345 - 当文件不存在时崩溃
if (!file.exists()) {
    return false;
}

// Fix: 使用 QueuedConnection 跨线程通信
// Bug: #12346 - Worker 线程直接更新 UI 导致崩溃
connect(worker, &Worker::progress, ui, &UI::updateProgress,
        Qt::QueuedConnection);
```

---

## 测试验证

### 单元测试

为修复添加回归测试：

```cpp
void TestBugFix::testFileNotExistence() {
    // Bug: #12345 - 文件不存在时崩溃
    QString content = FileService::getContent("/nonexistent/file");
    QCOMPARE(content, QString());
}
```

### 手动验证

- [ ] 按照 Bug 复现步骤验证
- [ ] 检查修复不引入新问题
- [ ] 验证相关功能正常

---

## 修复完成标准

- [ ] 根因分析完成（5-Why 表格）
- [ ] 修复方案用户确认
- [ ] 代码修改最小化
- [ ] 添加修复注释
- [ ] 编译通过，无警告
- [ ] 单元测试通过
- [ ] 手动验证修复效果
- [ ] 相关人员审查通过
