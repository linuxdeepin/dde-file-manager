// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersharemanager.h"
#include "dbusadaptor/usershare_adaptor.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>
#include "app/policykithelper.h"

QString UserShareManager::ObjectPath = "/com/deepin/filemanager/daemon/UserShareManager";
QString UserShareManager::PolicyKitActionId = "com.deepin.filemanager.daemon.UserShareManager";

UserShareManager::UserShareManager(QObject *parent)
    : QObject(parent)
    , QDBusContext()
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_userShareAdaptor = new UserShareAdaptor(this);
}

UserShareManager::~UserShareManager()
{

}

bool UserShareManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (c.isConnected()) {
        pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid) {
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);
    }

    if (!ret) {
        qDebug() << "Authentication failed !!";
    }
    return ret;
}

bool UserShareManager::addGroup(const QString &groupName)
{
    if (!checkAuthentication()) {
        qDebug() << "addGroup failed" <<  groupName;
        return false;
    }

    QStringList args;
    args << groupName;
    bool ret = QProcess::startDetached("groupadd", args);
    qDebug() << "groupadd" << groupName << ret;
    return ret;
}

bool UserShareManager::setUserSharePassword(const QString &username, const QString &passward)
{
    QByteArray encodedByteArray = username.toUtf8();
    QByteArray decodedByteArray = QByteArray::fromBase64(encodedByteArray);
    const QString &decodedUsername = QString::fromUtf8(decodedByteArray);

    encodedByteArray = passward.toUtf8();
    decodedByteArray = QByteArray::fromBase64(encodedByteArray);
    const QString &decodedPassward = QString::fromUtf8(decodedByteArray);

    if (!checkAuthentication()) {
        qDebug() << "setUserSharePassword failed" <<  decodedUsername;
        return false;
    }

    qDebug() << decodedUsername;// << passward; // log password?
    QStringList args;
    args << "-a" << decodedUsername << "-s";
    QProcess p;
    p.start("smbpasswd", args);
    p.write(decodedPassward.toStdString().c_str());
    p.write("\n");
    p.write(decodedPassward.toStdString().c_str());
    p.closeWriteChannel();
    bool ret = p.waitForFinished();
    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool UserShareManager::closeSmbShareByShareName(const QString &sharename, const bool bshow)
{
    if (!bshow) {
        return true;
    }
//    if (!checkAuthentication()) {
//        qDebug() << "closeSmbShareByShareName failed" <<  sharename;
//        return false;
//    }
    unsigned int suid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (!c.isConnected()) {
        qDebug() << "连接DBus失败";
        return false;
    }
    suid = c.interface()->serviceUid(message().service()).value(); //获取调用总线进程属主

    QString filename = sharename.toLower(); //文件名小写
    QFileInfo info("/var/lib/samba/usershares/" + filename);
    if (suid != info.ownerId() && suid != 0) { //对比文件属主与调用总线进程属主
        qDebug() << "非属主用户" << info.path();
        return  false;
    }

    QProcess p;
    //取得所有连击的pid
    QString cmd = QString("smbcontrol smbd close-share %1").arg(sharename);
    qDebug() << "cmd==========" << cmd;
    p.start(cmd);
    bool ret = p.waitForFinished();

    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool UserShareManager::createShareLinkFile()
{
    // 创建链接文件之前已经提权了 这里就不需要再次判断权限了
    /*if (!checkAuthentication()) {
        qDebug() << "createShareLinkFile";
        return false;
    }*/

    QProcess sh;
    static QString cmd("ln -sf /lib/systemd/system/smbd.service /etc/systemd/system/multi-user.target.wants/smbd.service");
    sh.start(cmd);
    auto ret = sh.waitForFinished();
    return ret;
}

bool UserShareManager::isUserSharePasswordSet(const QString &username)
{
    QProcess p;
    p.start("pdbedit -L");
    auto ret = p.waitForFinished();
    QStringList resultLines = QString::fromUtf8(p.readAllStandardOutput()).split('\n');
    bool isPasswordSet = false;
    foreach (const QString &line, resultLines) {
        if (line.startsWith(username + ":")) {
            isPasswordSet = true;
            break;
        }
    }
    return ret && isPasswordSet;
}
