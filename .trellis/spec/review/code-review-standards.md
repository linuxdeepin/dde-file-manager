# 代码审查标准

> DDE 桌面应用代码审查指南，专注于 C/C++/Qt 开发场景。

---

## 概览

本指南定义 DDE 代码审查的标准流程和检查清单。

---

## 审查流程

### 第 1 阶段: 上下文收集（2-3 分钟）

审查前，理解：
1. 阅读 PR/commit 描述和关联的 issue
2. 检查 PR 大小（超过 400 行？要求拆分）
3. 检查 CI/CD 状态（构建通过？）
4. 理解业务需求
5. 注意相关的架构决策

### 第 2 阶段: 高级审查（5-10 分钟）

1. **架构与设计**
   - 解决方案是否合适？
   - 检查：SOLID 原则、耦合/内聚、反模式
2. **性能评估**
   - 算法复杂度、内存使用、阻塞操作
3. **文件组织**
   - 新文件是否在正确的位置？
4. **测试策略**
   - 是否有测试覆盖边界情况？

### 第 3 阶段: 逐行审查（10-20 分钟）

对每个文件检查：
- **逻辑与正确性** - 边界情况、空检查、竞态条件
- **安全性** - 输入验证、注入风险、敏感数据
- **性能** - 不必要的循环、内存泄漏、阻塞 UI
- **可维护性** - 清晰命名、单一职责、注释

### 第 4 阶段: 总结与决策（2-3 分钟）

1. 总结关键问题
2. 突出你喜欢的部分
3. 做出明确决策：
   - ✅ 批准
   - 💬 评论（建议）
   - 🔄 请求更改（必须解决）

---

## C/C++ 审查清单

### 内存安全

```cpp
// ❌ 错误: 内存泄漏
QObject *obj = new QObject(); // 没有父对象！

// ✅ 正确: Parent-child 所有权
QObject *obj = new QObject(parent);

// ❌ 错误: Use after free
delete widget;
widget->show(); // 崩溃！

// ✅ 正确: 使用 deleteLater
widget->deleteLater();

// ❌ 错误: 缓冲区溢出
char buffer[10];
strcpy(buffer, "very long string"); // 溢出！

// ✅ 正确: 使用安全函数
strncpy(buffer, "string", sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

### 未定义行为

```cpp
// ❌ 错误: 整数溢出
int size = a + b; // 可能溢出

// ✅ 正确: 检查边界
if (a > INT_MAX - b) return error;
int size = a + b;

// ❌ 错误: 空指针解引用
QString name = obj->name(); // 如果 obj 为空会怎样？

// ✅ 正确: 空检查
if (!obj) return error;
QString name = obj->name();

// ❌ 错误: 未初始化变量
int value;
if (condition) value = 1;
// value 这里可能未初始化

// ✅ 正确: 初始化
int value = 0;
```

### RAII 模式

```cpp
// ❌ 错误: 手动资源管理
FILE *fp = fopen("file.txt", "r");
// ... 很多代码 ...
fclose(fp); // 错误时可能被跳过！

// ✅ 正确: RAII
std::ifstream file("file.txt");
// 作用域结束时自动关闭

// ✅ 正确: Qt 智能指针
QScopedPointer<Worker> worker(new Worker());
QSharedPointer<Resource> resource = QSharedPointer<Resource>::create();
```

---

## Qt 审查清单

### 信号槽连接

```cpp
// ❌ 错误: 跨线程没有 queued connection
connect(threadWorker, &Worker::signal, uiWidget, &Widget::slot);

// ✅ 正确: 跨线程使用 QueuedConnection
connect(threadWorker, &Worker::signal, uiWidget, &Widget::slot,
        Qt::QueuedConnection);

// ❌ 错误: 阻塞 UI 线程
QThread::sleep(5); // UI 冻结！

// ✅ 正确: 使用后台线程
QtConcurrent::run([this]() {
    // 耗时工作
});
```

### 对象生命周期

```cpp
// ❌ 错误: 在错误线程中删除 QObject
delete worker; // 如果 worker 在不同线程会崩溃！

// ✅ 正确: 使用 deleteLater
worker->deleteLater();

// ❌ 错误: Parent-child 违规
child->setParent(nullptr);
delete child; // 谁拥有它？

// ✅ 正确: 清晰的所有权
child->setParent(nullptr);
child->deleteLater();
```

### 内存泄漏

```cpp
// ❌ 错误: 没有父对象的 Widget
DWidget *widget = new DWidget(); // 泄漏！

// ✅ 正确: 有父对象的 Widget
DWidget *widget = new DWidget(parentWidget);

// ❌ 错误: 没有清理的 Timer
QTimer *timer = new QTimer();
timer->start(1000); // 泄漏！

// ✅ 正确: 有父对象的 Timer
QTimer *timer = new QTimer(this);
timer->start(1000);
```

---

## DDE 特定清单

### DTK 组件使用

| 禁止使用 | 改用 |
|---------|------|
| `QMainWindow` | `DMainWindow` |
| `QDialog` | `DDialog` |
| `QMessageBox` | `DMessageBox` |
| `QWidget` | `DWidget` |
| `QLabel` | `DLabel` |
| `QPushButton` | `DPushButton` |
| `QLineEdit` | `DLineEdit` |
| `QListView` | `DListView` |

### 主题适配

```cpp
// ❌ 错误: 硬编码颜色
label->setStyleSheet("color: #333333; background: #ffffff;");
button->setStyleSheet("background-color: #FF6B35;");

// ✅ 正确: 使用调色板
QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
label->setPalette(palette);

// ✅ 正确: 使用 DTK 样式
button->setButtonType(DPushButton::RecommendButton);
```

### D-Bus 集成

```cpp
// ❌ 错误: 错误的总线类型
QDBusConnection::sessionBus().connect(...); // 应该用 systemBus？

// ✅ 正确: 正确的总线类型
// 系统服务: systemBus
QDBusConnection::systemBus().connect(
    "org.freedesktop.UPower", ...);

// 用户服务: sessionBus
QDBusConnection::sessionBus().connect(...);

// ❌ 错误: UI 线程阻塞调用
QDBusInterface iface(...);
QDBusReply<QString> reply = iface.call("SlowMethod"); // 阻塞 UI！

// ✅ 正确: 异步调用
QDBusPendingCall async = iface.asyncCall("SlowMethod");
QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
connect(watcher, &QDBusPendingCallWatcher::finished,
        this, &MyClass::handleResult);
```

### Polkit 认证

```cpp
// ❌ 错误: 使用 sudo
system("sudo rm -rf /important"); // 安全风险！

// ✅ 正确: 使用 Polkit
PolkitQt1::Authority::instance()->checkAuthorizationSync(
    "org.deepin.myapp.authentication",
    PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
    PolkitQt1::Authority::AllowUserInteraction);
```

---

## 安全检查清单

### 输入验证

```cpp
// ❌ 错误: 没有验证
QFile file(userInput); // 路径遍历风险！

// ✅ 正确: 验证输入
QString sanitized = sanitizePath(userInput);
if (!isPathSafe(sanitized)) return;
QFile file(sanitized);

// ❌ 错误: SQL 注入
QString query = "SELECT * FROM users WHERE name = '" + name + "'";

// ✅ 正确: 使用预处理语句
QSqlQuery query;
query.prepare("SELECT * FROM users WHERE name = ?");
query.addBindValue(name);
```

### 敏感数据

```cpp
// ❌ 错误: 记录敏感数据
qDebug() << "Password:" << password;

// ✅ 正确: 掩码敏感数据
qDebug() << "User authenticated";

// ❌ 错误: 明文存储密码
settings.setValue("password", password);

// ✅ 正确: 使用密钥环
// 使用 libsecret 或 KWallet 进行安全存储
```

---

## 性能检查清单

### 常见问题

```cpp
// ❌ 错误: N+1 查询
for (const auto &item : items) {
    db.query("SELECT * FROM details WHERE id = " + item.id);
}

// ✅ 正确: 批量查询
QStringList ids;
for (const auto &item : items) ids << QString::number(item.id);
db.query("SELECT * FROM details WHERE id IN (" + ids.join(",") + ")");

// ❌ 错误: UI 线程繁重工作
void onButtonClick() {
    processLargeFile(); // UI 冻结！
}

// ✅ 正确: 使用后台线程
void onButtonClick() {
    QtConcurrent::run([this]() {
        processLargeFile();
    });
}
```

---

## 严重性标签

| 标签 | 含义 | 示例 |
|-----|------|------|
| 🔴 `blocking` | 合并前必须修复 | 内存泄漏、安全问题、崩溃 |
| 🟡 `important` | 应该修复 | 性能问题、代码异味 |
| 🟢 `nit` | 次要样式问题 | 变量命名、空格 |
| 💡 `suggestion` | 可选改进 | 更好的算法建议 |
| 🎉 `praise` | 做得好 | 结构良好的代码 |

---

## 审查反馈模板

```markdown
## 代码审查总结

**文件变更**: X 个文件, +Y/-Z 行

### 🔴 阻塞问题
1. [file:line] 关键问题描述

### 🟡 重要问题
1. [file:line] 重要问题描述

### 💡 建议
1. [file:line] 可选改进

### 🎉 做得好
- [file] 中出色的错误处理
- [class] 的清晰抽象

**决策**: 🔄 请求更改 / ✅ 批准 / 💬 评论
```

---

## 快速参考

| 检查 | 命令/方法 |
|------|----------|
| 构建错误 | 变更文件上的 `lsp_diagnostics` |
| 内存泄漏 | 检查没有父对象 `new`、没有 `free` 的 `malloc` |
| 线程安全 | 检查跨线程的 `Qt::QueuedConnection` |
| 主题支持 | 检查没有硬编码颜色 |
| DTK 组件 | 检查没有 `QMainWindow`、`QDialog` 等 |
| D-Bus 正确 | 检查 systemBus vs sessionBus |
| 安全性 | 检查输入验证、日志中没有敏感数据 |
