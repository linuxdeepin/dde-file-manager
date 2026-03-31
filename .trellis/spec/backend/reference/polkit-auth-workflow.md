# Polkit 鉴权工作流

## 核心原则

**使用 Polkit 进行权限提升**，禁止以 root 身份运行整个应用。

## 何时使用 Polkit

| 场景 | 使用 | 原因 |
|------|------|------|
| 修改 `/etc` 配置 | ✅ Polkit | 系统级操作需要鉴权 |
| 安装/卸载软件 | ✅ Polkit | 系统级操作 |
| 修改系统服务 | ✅ Polkit | 系统级操作 |
| 访问硬件设备 | ✅ Polkit | 需要特殊权限 |
| 普通文件操作 | ❌ 不需要 | 用户权限即可 |
| 应用配置修改 | ❌ 不需要 | 用户主目录内操作 |

## Polkit 架构

```
┌─────────────────┐
│  你的应用       │
│  (非特权进程)   │
└────────┬────────┘
         │ 请求授权
         ↓
┌─────────────────┐
│  Polkit Agent   │
│  (认证对话框)   │
│  由 DDE 提供    │
└────────┬────────┘
         │ 用户输入密码
         ↓
┌─────────────────┐
│  Polkit Daemon  │
│  (系统服务)     │
│  验证凭据       │
└────────┬────────┘
         │ 授权结果
         ↓
┌─────────────────┐
│  Helper 程序    │
│  (以 root 运行) │
│  执行特权操作   │
└─────────────────┘
```

## 方法 1: PolkitQt5-1 直接鉴权

### CMake 配置

```cmake
find_package(PolkitQt5-1 REQUIRED)
# 或 Qt6
find_package(PolkitQt6-1 REQUIRED)

target_link_libraries(app PRIVATE PolkitQt5-1::PolkitQt5-1)
```

### 基本用法

```cpp
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

using namespace PolkitQt1;

class PrivilegedOperation : public QObject {
    Q_OBJECT
    
public:
    explicit PrivilegedOperation(QObject *parent = nullptr) 
        : QObject(parent) {}
    
    void performOperation() {
        // 检查权限
        Authority::instance()->checkAuthorizationAsync(
            "org.deepin.dde.policy.authentication",
            UnixProcessSubject(QCoreApplication::applicationPid()),
            Authority::AllowUserInteraction,
            this,
            SLOT(authorizationResult(PolkitQt1::Authority::Result))
        );
    }
    
private slots:
    void authorizationResult(Authority::Result result) {
        switch (result) {
        case Authority::Yes:
            qInfo() << "Authorization granted";
            performPrivilegedAction();
            break;
        case Authority::No:
            qWarning() << "Authorization denied";
            emit authorizationFailed("Permission denied");
            break;
        case Authority::Cancelled:
            qWarning() << "Authorization cancelled by user";
            emit authorizationFailed("Cancelled");
            break;
        }
    }
    
    void performPrivilegedAction() {
        // 执行需要特权的操作
        // 注意：这里仍然需要适当的权限机制
    }
    
signals:
    void authorizationFailed(const QString &reason);
};
```

## 方法 2: Helper 程序 + pkexec (推荐)

### 定义 Polkit Action

创建 `/usr/share/polkit-1/actions/com.example.myapp.policy`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<policyconfig>
  <vendor>Deepin</vendor>
  <vendor_url>https://www.deepin.org</vendor_url>
  <icon_name>deepin-editor</icon_name>
  
  <action id="com.example.myapp.modify-system-config">
    <description>Modify system configuration</description>
    <message>Authentication is required to modify system configuration</message>
    <defaults>
      <allow_any>auth_admin</allow_any>
      <allow_inactive>auth_admin</allow_inactive>
      <allow_active>auth_admin_keep</allow_active>
    </defaults>
    <annotate key="org.freedesktop.policykit.exec.path">/usr/lib/myapp/myapp-helper</annotate>
    <annotate key="org.freedesktop.policykit.exec.allow_gui">true</annotate>
  </action>
</policyconfig>
```

### Helper 程序

```cpp
// myapp-helper.cpp
// 以 root 身份运行，执行特权操作

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 检查是否以 root 运行
    if (geteuid() != 0) {
        qCritical() << "Helper must run as root";
        return 1;
    }
    
    // 从 stdin 读取命令
    QTextStream in(stdin);
    QString command = in.readLine();
    
    if (command == "write-config") {
        QString path = in.readLine();
        QString content = in.readLine();
        
        // 验证路径（防止目录遍历攻击）
        if (!path.startsWith("/etc/myapp/")) {
            qCritical() << "Invalid path:" << path;
            return 1;
        }
        
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(content.toUtf8());
            file.close();
            qDebug() << "OK";
            return 0;
        } else {
            qCritical() << "Failed to write:" << file.errorString();
            return 1;
        }
    }
    
    qCritical() << "Unknown command:" << command;
    return 1;
}
```

### 主程序调用 Helper

```cpp
#include <QProcess>
#include <QStandardPaths>

class SystemConfigManager : public QObject {
    Q_OBJECT
    
public:
    void writeConfig(const QString &path, const QString &content) {
        QProcess *process = new QProcess(this);
        
        // 查找 helper 程序
        QString helperPath = QStandardPaths::findExecutable("myapp-helper", 
            QStringList() << "/usr/lib/myapp" << "/usr/libexec/myapp");
        
        if (helperPath.isEmpty()) {
            emit configWriteFailed("Helper program not found");
            return;
        }
        
        // 使用 pkexec 启动 helper (会触发 Polkit 认证)
        process->start("pkexec", QStringList() << helperPath);
        
        // 发送命令和参数
        process->write("write-config\n");
        process->write(path.toUtf8() + "\n");
        process->write(content.toUtf8() + "\n");
        process->closeWriteChannel();
        
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, process](int exitCode, QProcess::ExitStatus status) {
            if (exitCode == 0) {
                QString output = QString::fromUtf8(process->readAllStandardOutput());
                if (output.trimmed() == "OK") {
                    emit configWritten();
                } else {
                    emit configWriteFailed(output);
                }
            } else {
                QString error = QString::fromUtf8(process->readAllStandardError());
                emit configWriteFailed(error.isEmpty() ? "Unknown error" : error);
            }
            process->deleteLater();
        });
        
        connect(process, &QProcess::errorOccurred,
                this, [this, process](QProcess::ProcessError error) {
            emit configWriteFailed(QString("Process error: %1").arg(error));
            process->deleteLater();
        });
    }
    
signals:
    void configWritten();
    void configWriteFailed(const QString &error);
};
```

## CMake 安装规则

```cmake
# 安装 helper 程序
install(TARGETS myapp-helper
    DESTINATION lib/myapp
)

# 安装 Polkit action 文件
install(FILES com.example.myapp.policy
    DESTINATION share/polkit-1/actions
)
```

## 常见错误

| 错误 | 后果 | 修复 |
|------|------|------|
| `sudo ./app` 启动 | 安全风险、文件权限问题 | → Polkit + Helper |
| 整个应用以 root 运行 | 安全隐患、用户文件损坏 | → 仅 Helper 提权 |
| 不验证 Helper 输入 | 命令注入风险 | → 严格验证路径和参数 |
| 缺少 Polkit action | 认证失败 | → 创建 `.policy` 文件 |
| Helper 路径错误 | pkexec 失败 | → 使用绝对路径 |

## 安全最佳实践

1. **最小权限原则**: Helper 只执行必要的特权操作
2. **输入验证**: 严格验证所有传入参数
3. **路径限制**: 限制可访问的目录范围
4. **日志记录**: 记录所有特权操作
5. **错误处理**: 不泄露敏感信息

## 调试

```bash
# 检查 Polkit 服务状态
systemctl status polkit

# 查看 Polkit 日志
journalctl -u polkit -f

# 测试 action 定义
pkcheck --action-id com.example.myapp.modify-system-config --process $$

# 查看已注册的 actions
ls -la /usr/share/polkit-1/actions/
```

## 相关资源

- [Polkit 文档](https://www.freedesktop.org/software/polkit/docs/latest/)
- [PolkitQt5-1 API](https://github.com/linuxdeepin/polkit-qt5-1)
- [Deepin Polkit Agent](https://github.com/linuxdeepin/deepin-polkit-agent)
