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

