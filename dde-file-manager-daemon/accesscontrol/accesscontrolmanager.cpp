#include "accesscontrolmanager.h"
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
#include "dbusservice/dbusadaptor/accesscontrol_adaptor.h"

QString AccessControlManager::ObjectPath = "/com/deepin/filemanager/daemon/AccessControlManager";
QString AccessControlManager::PolicyKitActionId = "com.deepin.filemanager.daemon.AccessControlManager";

AccessControlManager::AccessControlManager(QObject *parent)
    : QObject(parent)
    , QDBusContext(),
      m_watcher(new DFileSystemWatcher(this))
{
    qDebug() << "register:" << ObjectPath;
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("=======AccessControlManager Register Object Failed.");
    }
    m_diskMnanager = new DDiskManager(this);
    m_diskMnanager->setWatchChanges(true);
    qDebug() << "=======AccessControlManager() ";

    m_watcher->addPath("/home");
    onFileCreated("/home", "root");
    initConnect();
}

AccessControlManager::~AccessControlManager()
{
    qDebug() << "~AccessControlManager()";
}


void AccessControlManager::initConnect()
{
    qDebug() << "AccessControlManager::initConnect()";
    connect(m_diskMnanager, &DDiskManager::mountAdded, this, &AccessControlManager::chmodMountpoints);
    connect(m_watcher, &DFileSystemWatcher::fileCreated, this, &AccessControlManager::onFileCreated);
}

bool AccessControlManager::checkAuthentication()
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


void AccessControlManager::chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    Q_UNUSED(blockDevicePath);
    qDebug() << "chmod ==>" << mountPoint;
    struct stat fileStat;
    stat(mountPoint.data(), &fileStat);
    chmod(mountPoint.data(), (fileStat.st_mode | S_IWUSR | S_IWGRP | S_IWOTH));
#if 0
    // system call
    struct mntent *ent = NULL;
    FILE *aFile = NULL;

    aFile = setmntent("/proc/mounts", "r");
    if (aFile == NULL) {
      perror("setmntent()");
      return;
    }
    while (NULL != (ent = getmntent(aFile))) {
        QString fsName(ent->mnt_fsname);
        QString mntDir(ent->mnt_dir);
        if (fsName.startsWith("/")) {
            qDebug() << "mount fs name: " << fsName << ", mount path:" << mntDir;
            struct stat fileStat;
            stat(ent->mnt_dir, &fileStat);
            if (chmod(ent->mnt_dir, (fileStat.st_mode | S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
                qDebug() << "chmod " << mntDir << "success";
            } else {
                qDebug() << "chmod " << mntDir << "faild";
            }
        }
    }
    endmntent(aFile);
#endif
}


void AccessControlManager::onFileCreated(const QString &path, const QString &name)
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
