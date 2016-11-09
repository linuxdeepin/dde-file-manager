#include "usersharemanager.h"
#include "dbusservice/dbusadaptor/usershare_adaptor.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>


QString UserShareManager::ObjectPath = "/com/deepin/filemanager/daemon/UserShareManager";

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_userShareAdaptor = new UserShareAdaptor(this);
}

UserShareManager::~UserShareManager()
{

}

bool UserShareManager::addGroup(const QString &groupName)
{
    QStringList args;
    args << groupName;
    bool ret = QProcess::startDetached("/usr/sbin/groupadd", args);
    qDebug() << "groupadd" << groupName << ret;
    return ret;
}

bool UserShareManager::addUserToGroup(const QString &userName, const QString &groupName)
{
    addGroup(groupName);
    QStringList args;
    args << userName << groupName;
    bool ret = QProcess::startDetached("/usr/sbin/adduser", args);
    qDebug() << "adduser" << userName << groupName << ret;
    return ret;
}

bool UserShareManager::setUserSharePassword(const QString &username, const QString &passward)
{
    qDebug() << username << passward;
    QStringList args;
    args << "-a" << username << "-s";
    QProcess p;
    p.start("smbpasswd", args);
    p.write(passward.toStdString().c_str());
    p.write("\n");
    p.write(passward.toStdString().c_str());
    p.closeWriteChannel();
    bool ret = p.waitForFinished();
    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool UserShareManager::restartSambaService()
{
    QStringList args;
    args << "restart";
    bool ret = QProcess::startDetached("/usr/sbin/smbd", args);
    qDebug() << "smbd restart" << ret;
    return ret;
}

