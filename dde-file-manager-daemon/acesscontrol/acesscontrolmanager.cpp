#include "acesscontrolmanager.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "dfilesystemwatcher.h"

#include "app/policykithelper.h"
#include "dbusservice/dbusadaptor/acesscontrol_adaptor.h"

QString AcessControlManager::ObjectPath = "/com/deepin/filemanager/daemon/AcessControlManager";
QString AcessControlManager::PolicyKitActionId = "com.deepin.filemanager.daemon.AcessControlManager";

AcessControlManager::AcessControlManager(QObject *parent)
    : QObject(parent)
    , QDBusContext(),
      m_watcher(new DFileSystemWatcher(this))
{
    qDebug() << "register:" << ObjectPath;
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("AcessControlManager Register Object Failed.");
    }
    m_diskMnanager = new DDiskManager(this);
    m_diskMnanager->setWatchChanges(true);

    m_watcher->addPath("/home");
    onFileCreated("/home", "root");
    initConnect();
}

AcessControlManager::~AcessControlManager()
{
}


void AcessControlManager::initConnect()
{
    connect(m_watcher, &DFileSystemWatcher::fileCreated, this, &AcessControlManager::onFileCreated);
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


void AcessControlManager::onFileCreated(const QString &path, const QString &name)
{
    Q_UNUSED(path)
    Q_UNUSED(name)

    QDir homeDir("/home");
    QStringList dirNames = homeDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirNames.append("root"); // root 用户不在/home下，但设备会挂载到/media/root
    for (const QString &dirName : dirNames) {
        // /media/[UserName] 为默认挂载的基路径，预先从创建此目录，目的是为了确保该路径其他用户能够访问
        QString mountBaseName = QString("/media/%1").arg(dirName);
        QDir mountDir(mountBaseName);
        if (!mountDir.exists()) {
            qDebug() << mountBaseName << "not exists";
            if (QDir().mkpath(mountBaseName)) {
                qDebug() << "create" << mountBaseName << "success";
                struct stat fileStat;
                stat(mountBaseName.toUtf8().data(), &fileStat);
                chmod(mountBaseName.toUtf8().data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
            }
        }
        // ACL
        QString aclCmd = QString("setfacl -m o:rx %1").arg(mountBaseName);
        QProcess::execute(aclCmd);
        qDebug() << "acl:" << aclCmd;
    }

}
