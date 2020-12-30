#ifndef ACESSCONTROLMANAGER_H
#define ACESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AcessControlAdaptor;
class DDiskManager;
class DFileSystemWatcher;

class AcessControlManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.AcessControlManager")

public:
    explicit AcessControlManager(QObject *parent = nullptr);
    ~AcessControlManager();

    void initConnect();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();

private slots:
    void onFileCreated(const QString &path, const QString &name);

private:
    AcessControlAdaptor *m_acessControlAdaptor = nullptr;
    DDiskManager *m_diskMnanager = nullptr;
    DFileSystemWatcher *m_watcher = nullptr;
};

#endif // ACESSCONTROLMANAGER_H
