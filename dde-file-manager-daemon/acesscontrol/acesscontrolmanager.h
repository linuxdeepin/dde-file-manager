#ifndef ACESSCONTROLMANAGER_H
#define ACESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AcessControlAdaptor;
class DDiskManager;

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
signals:

public slots:
    bool acquireFullAuthentication(const QString &userName, const QString &path);
    void chmodMountpoints();

private:
    AcessControlAdaptor* m_acessControlAdaptor = nullptr;
    DDiskManager* m_diskMnanager = nullptr;
};

#endif // ACESSCONTROLMANAGER_H
