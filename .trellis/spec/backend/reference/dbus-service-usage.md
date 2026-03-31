# DBus 服务使用指南

## 核心原则

**使用 DBus 事件驱动获取系统状态**，不要轮询 sysfs 文件。

## 何时使用 DBus

| 场景 | 使用 | 原因 |
|------|------|------|
| 电池/电源状态 | ✅ DBus (UPower) | 事件驱动，实时通知 |
| 网络连接状态 | ✅ DBus (NetworkManager) | 事件驱动，状态完整 |
| 显示器亮度 | ✅ DBus (DDE Display) | 系统统一管理 |
| 音量控制 | ✅ DBus (DDE Audio) | 系统统一管理 |
| 磁盘信息 | ⚠️ DBus 或 udisks2 | 两者都可以 |
| 简单系统信息 | ⚠️ sysfs 或 DBus | 静态信息可轮询 |

## 常用 DDE/FreeDesktop 服务

| 服务 | 总线 | 用途 |
|------|------|------|
| `org.freedesktop.UPower` | System | 电池、电源事件 |
| `org.freedesktop.NetworkManager` | System | 网络连接状态 |
| `org.deepin.dde.Display1` | Session | 显示器设置、亮度 |
| `org.deepin.dde.Audio1` | Session | 音量、音频设备 |
| `org.deepin.dde.SystemInfo1` | Session | 系统信息 |
| `org.freedesktop.UDisks2` | System | 磁盘、挂载 |

## 基本用法

### 监听电源事件 (UPower)

```cpp
#include <QDBusInterface>
#include <QDBusConnection>

class PowerManager : public QObject {
    Q_OBJECT
    
public:
    PowerManager(QObject *parent = nullptr) : QObject(parent) {
        // 连接 UPower 服务
        m_upowerInterface = new QDBusInterface(
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.UPower",
            QDBusConnection::systemBus(),
            this
        );
        
        // 监听属性变化
        QDBusConnection::systemBus().connect(
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this,
            SLOT(handlePropertiesChanged(QString, QVariantMap, QStringList))
        );
        
        // 获取初始状态
        enumerateDevices();
    }
    
    void enumerateDevices() {
        QDBusReply<QList<QDBusObjectPath>> reply = 
            m_upowerInterface->call("EnumerateDevices");
        
        if (reply.isValid()) {
            for (const QDBusObjectPath &path : reply.value()) {
                readDeviceProperties(path.path());
            }
        }
    }
    
    void readDeviceProperties(const QString &path) {
        QDBusInterface device(
            "org.freedesktop.UPower",
            path,
            "org.freedesktop.DBus.Properties",
            QDBusConnection::systemBus()
        );
        
        QDBusReply<QVariantMap> props = device.call("GetAll", 
            "org.freedesktop.UPower.Device");
        
        if (props.isValid()) {
            QVariantMap properties = props.value();
            
            uint state = properties["State"].toUInt();
            double percentage = properties["Percentage"].toDouble();
            bool present = properties["IsPresent"].toBool();
            
            qInfo() << "Battery:" << percentage << "%" 
                    << "State:" << state
                    << "Present:" << present;
        }
    }
    
private slots:
    void handlePropertiesChanged(const QString &interface,
                                 const QVariantMap &changed,
                                 const QStringList &invalidated) {
        if (interface == "org.freedesktop.UPower.Device") {
            for (auto it = changed.begin(); it != changed.end(); ++it) {
                if (it.key() == "Percentage") {
                    emit batteryLevelChanged(it.value().toDouble());
                }
                if (it.key() == "State") {
                    emit powerStateChanged(it.value().toUInt());
                }
            }
        }
    }
    
signals:
    void batteryLevelChanged(double percentage);
    void powerStateChanged(uint state);
    
private:
    QDBusInterface *m_upowerInterface;
};
```

### 监听网络状态 (NetworkManager)

```cpp
#include <QDBusInterface>
#include <QDBusConnection>

class NetworkMonitor : public QObject {
    Q_OBJECT
    
public:
    NetworkMonitor(QObject *parent = nullptr) : QObject(parent) {
        m_nmInterface = new QDBusInterface(
            "org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager",
            QDBusConnection::systemBus(),
            this
        );
        
        // 监听状态变化
        QDBusConnection::systemBus().connect(
            "org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager",
            "PropertiesChanged",
            this,
            SLOT(handleNetworkStateChanged(QVariantMap))
        );
        
        // 获取初始状态
        getNetworkState();
    }
    
    void getNetworkState() {
        QDBusReply<uint> stateReply = m_nmInterface->call("state");
        
        if (stateReply.isValid()) {
            uint state = stateReply.value();
            
            // NM 状态枚举
            // NM_STATE_CONNECTED_GLOBAL = 70
            // NM_STATE_CONNECTED_SITE = 60
            // NM_STATE_DISCONNECTED = 20
            
            qInfo() << "Network state:" << state;
            emit networkStateChanged(state);
        }
    }
    
private slots:
    void handleNetworkStateChanged(const QVariantMap &properties) {
        if (properties.contains("State")) {
            uint state = properties["State"].toUInt();
            emit networkStateChanged(state);
        }
    }
    
signals:
    void networkStateChanged(uint state);
    
private:
    QDBusInterface *m_nmInterface;
};
```

### 控制显示器亮度 (DDE)

```cpp
#include <QDBusInterface>
#include <QDBusConnection>

class DisplayController : public QObject {
    Q_OBJECT
    
public:
    DisplayController(QObject *parent = nullptr) : QObject(parent) {
        m_displayInterface = new QDBusInterface(
            "org.deepin.dde.Display1",
            "/org/deepin/dde/Display1",
            "org.deepin.dde.Display1",
            QDBusConnection::sessionBus(),
            this
        );
    }
    
    double getBrightness() {
        QDBusReply<double> reply = m_displayInterface->property("Brightness");
        return reply.isValid() ? reply.value() : 0.5;
    }
    
    void setBrightness(double value) {
        m_displayInterface->call("SetBrightness", qBound(0.0, value, 1.0));
    }
    
signals:
    void brightnessChanged(double value);
    
private:
    QDBusInterface *m_displayInterface;
};
```

### 控制音量 (DDE Audio)

```cpp
#include <QDBusInterface>
#include <QDBusConnection>

class AudioController : public QObject {
    Q_OBJECT
    
public:
    AudioController(QObject *parent = nullptr) : QObject(parent) {
        m_audioInterface = new QDBusInterface(
            "org.deepin.dde.Audio1",
            "/org/deepin/dde/Audio1/Sink",
            "org.deepin.dde.Audio1.Sink",
            QDBusConnection::sessionBus(),
            this
        );
    }
    
    double getVolume() {
        QDBusReply<double> reply = m_audioInterface->property("Volume");
        return reply.isValid() ? reply.value() : 0.5;
    }
    
    void setVolume(double value) {
        m_audioInterface->call("SetVolume", qBound(0.0, value, 1.0), true);
    }
    
    bool isMuted() {
        QDBusReply<bool> reply = m_audioInterface->property("Mute");
        return reply.isValid() && reply.value();
    }
    
    void setMute(bool muted) {
        m_audioInterface->call("SetMute", muted);
    }
    
private:
    QDBusInterface *m_audioInterface;
};
```

## 错误处理

```cpp
QDBusInterface interface("org.example.Service", "/org/example/Path");

if (!interface.isValid()) {
    qWarning() << "DBus interface invalid:" 
               << QDBusConnection::sessionBus().lastError().message();
    return;
}

QDBusReply<QVariant> reply = interface.call("MethodName");

if (!reply.isValid()) {
    qWarning() << "DBus call failed:" << reply.error().message();
    return;
}
```

## 常见错误

| 错误 | 后果 | 修复 |
|------|------|------|
| 轮询 `/sys/class/power_supply` | CPU 浪费、延迟 | → DBus 信号监听 |
| 使用错误的总线 | 连接失败 | → System vs Session |
| 不检查接口有效性 | 崩溃 | → `isValid()` 检查 |
| 忽略错误处理 | 无法诊断 | → 检查 `QDBusError` |
| 同步调用阻塞 UI | UI 冻结 | → 异步调用或后台线程 |

## CMake 配置

```cmake
find_package(Qt6 REQUIRED COMPONENTS DBus)
# 或
find_package(Qt5 REQUIRED COMPONENTS DBus)

target_link_libraries(app PRIVATE Qt6::DBus)
```

## 相关资源

- [Qt DBus 文档](https://doc.qt.io/qt-6/qtdbus-index.html)
- [FreeDesktop DBus API](https://www.freedesktop.org/wiki/Software/dbus/)
- [UPower DBus API](https://upower.freedesktop.org/docs/)
