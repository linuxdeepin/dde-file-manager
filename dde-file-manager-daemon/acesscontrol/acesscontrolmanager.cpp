#include "acesscontrolmanager.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "app/policykithelper.h"
#include "dbusservice/dbusadaptor/acesscontrol_adaptor.h"

QString AcessControlManager::ObjectPath = "/com/deepin/filemanager/daemon/AcessControlManager";
QString AcessControlManager::PolicyKitActionId = "com.deepin.filemanager.daemon.AcessControlManager";

AcessControlManager::AcessControlManager(QObject *parent)
    : QObject(parent)
    , QDBusContext()
{
    qDebug() << "register:" << ObjectPath;
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("=======AcessControlManager Register Object Failed.");
    }
    m_acessControlAdaptor = new AcessControlAdaptor(this);
    qDebug() << "=======AcessControlManager() ";

}

AcessControlManager::~AcessControlManager()
{

}

bool AcessControlManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected()) {
       pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid){
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);
    }

    if (!ret) {
        qDebug() << "Authentication failed !!";
        qDebug() << "failed pid: " << pid;
        qDebug() << "failed policy id:" << PolicyKitActionId;
    }
    return ret;
}

bool AcessControlManager::acquireFullAuthentication(const QString &userName, const QString &path)
{
    Q_UNUSED(userName)

    bool ret = false;
    QByteArray pathBytes(path.toLocal8Bit());
    struct stat fileStat;
    stat(pathBytes.data(), &fileStat);
    if (chmod(pathBytes.data(), (fileStat.st_mode | S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
        qDebug() << "chmod() success!";
        ret = true;
    } else {
        qDebug() << "chmod() failed";
        if (!checkAuthentication()) {
            qDebug() << "acquireFullAuthentication failed";
            return false;
        }
        qDebug() << "acquireFullAuthentication success";
        QProcess p;
        QString cmd = QString("chmod 777 %1").arg(path);
        qDebug() << "cmd1==========" << cmd;
        p.start(cmd);
        ret = p.waitForFinished();
        if (!ret) {
            qDebug() << "exec cmd1 failed";
        }

        qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    }

    return ret;
}
