#ifndef ACCESSCONTROLMANAGER_H
#define ACCESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AccessControlAdaptor;
class DDiskManager;
class DFileSystemWatcher;

class AccessControlManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.AccessControlManager")

public:
    explicit AccessControlManager(QObject *parent = nullptr);
    ~AccessControlManager();

    void initConnect();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();
signals:


private slots:
    void onFileCreated(const QString &path, const QString &name);
    void chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint);

private:
    AccessControlAdaptor *m_accessControlAdaptor = nullptr;
    DDiskManager *m_diskMnanager = nullptr;
    DFileSystemWatcher *m_watcher = nullptr;
};

#endif // ACCESSCONTROLMANAGER_H
