# Dbus 异步/同步问题

> 使用错误的 DBus 调用方式导致阻塞 UI 或数据不一致。

---

## 问题描述 (P1)

在 DDE 应用中使用 DBus 时：
- 阻塞同步调用导致 UI 冻结
- 异步调用结果未正确处理
- 跨线程通信使用错误的连接类型

---

## 根本原因

1. 混淆同步/异步调用时机
2. 系统服务调用未考虑网络延迟
3. 信号槽连接类型不当

---

## 错误示例

### 1. 同步阻塞调用

```cpp
// ❌ 错误: UI 线程阻塞调用
void checkBattery() {
    QDBusInterface iface("org.freedesktop.UPower",
                         "/org/freedesktop/UPower");
    // 阻塞 UI 线程！
    QDBusReply<QVariant> reply = iface.call("GetAll");
    processBatteryLevel(reply.value());
}
```

### 2. 错误的总线类型

```cpp
// ❌ 错误: 系统服务使用 sessionBus
void getBrightness() {
    // 错误: Power 属于系统服务
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Power",
        "/org/freedesktop/Power",
        ...);
    // 连接失败！
}
```

### 3. 异步调用无错误处理

```cpp
// ❌ 错误: 异步调用没有错误处理
void asyncCall() {
    QDBusInterface iface(...);
    iface.asyncCall("Method");  // 没有监听结果或错误
}
```

---

## 正确做法

### 使用异步调用

```cpp
void checkBattery() {
    QDBusInterface iface("org.freedesktop.UPower",
                         "/org/freedesktop/UPower");
    QDBusPendingCall async = iface.asyncCall("GetAll");

    // 设置监听
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &MyClass::onBatteryUpdated);
}

void onBatteryUpdated(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QVariantMap> reply = *watcher;
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        return;
    }

    QVariantMap values = reply.value();
    updateUI(values);
    watcher->deleteLater();
}
```

### 正确的总线类型

```cpp
// ✅ 正确: 系统服务使用 systemBus
void monitorPower() {
    // ✅ 系统级服务
    QDBusConnection::systemBus().connect(
        "org.freedesktop.UPower",
        "/org/freedesktop/UPower",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this, SLOT(handlePropertyChange(QString, QVariantMap)));
}

// ✅ 正确: 用户服务使用 sessionBus
void monitorClipboard() {
    QDBusConnection::sessionBus().connect(
        "org.deepin.dde.Clipboard",
        "/org/deepin/dde/Clipboard",
        ...);
}
```

### 异步调用错误处理

```cpp
void asyncCall() {
    QDBusPendingCall async = iface.asyncCall("Method", arg1, arg2);

    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, watcher]() {
                QDBusPendingReply<QString> reply = *watcher;
                if (reply.isError()) {
                    handleError(reply.error());
                } else {
                    handleResult(reply.value());
                }
                watcher->deleteLater();
            });
}
```

---

## DBus 调用流程

### 同步调用（仅后台线程）

```cpp
// 仅在非 UI 线程中使用
void workerFunction() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface iface(...);

    // 后台线程可以同步调用
    QDBusReply<QString> reply = iface.call("Method");
    if (reply.isValid()) {
        process(reply.value());
    }
}
```

### 异步调用（UI 线程）

```cpp
// UI 线程必须使用异步调用
void uiFunction() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface iface(...);

    // 异步调用
    QDBusPendingCall async = iface.asyncCall("Method");
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &MyClass::onResult);
}
```

---

## 常见服务总线类型

| 服务类型 | 总线 | 示例 |
|---------|------|------|
| 系统服务 | `systemBus` | UPower, NetworkManager, Polkit, systemd |
| 用户服务 | `sessionBus` | Clipboard, Settings, App Services |

---

## 检测方法

### 检查 DBus 连接

```cpp
bool checkDBusConnection() {
    QDBusConnection sys = QDBusConnection::systemBus();
    QDBusConnection sess = QDBusConnection::sessionBus();

    return sys.isConnected() && sess.isConnected();
}
```

### 记录 DBus 调用耗时

```cpp
void callDBusMethod() {
    QElapsedTimer timer;
    timer.start();

    QDBusReply reply = iface.call("Method");

    qint64 elapsed = timer.elapsed();
    if (elapsed > 100) {  // 超过 100ms 记录警告
        qWarning() << "DBus call took" << elapsed << "ms";
    }
}
```

---

## 预防措施

### 1. UI 线程异步调用

```cpp
// 规则: UI 线程所有 DBus 调用使用 asyncCall
// 后台线程可以同步调用，但推荐也用异步
```

### 2. 总线类型命名约定

```cpp
// 清晰命名连接
class DbusService {
    QDBusConnection systemBus() const {
        return QDBusConnection::systemBus();
    }

    QDBusConnection sessionBus() const {
        return QDBusConnection::sessionBus();
    }
};
```

### 3. 超时设置

```cpp
QDBusPendingCall async = iface.asyncCall("Method");
async.setTimeout(5000);  // 5 秒超时
```

---

## 代码检查清单

- [ ] UI 线程中使用 asyncCall 而非 call
- [ ] 系统服务使用 systemBus
- [ ] 用户服务使用 sessionBus
- [ ] 所有异步调用监听完成和错误
- [ ] 设置合理的超时时间
- [ ] 记录慢调用（>100ms）
- [ ] 处理 DBus 连接失败情况
