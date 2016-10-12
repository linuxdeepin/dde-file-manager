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

bool UserShareManager::setUserSharePassword(const QString &username, const QString &passward)
{
    qDebug() << username << passward;
    QString cmd = QString("echo -e \"%1\n%2\" | smbpasswd -a -s %3").arg(passward, passward, username);
    qDebug() << cmd;
    bool ret = QProcess::startDetached(cmd);
    return ret;
}

