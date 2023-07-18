// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecontroldbus.h"
#include "polkit/policykithelper.h"
#include "dbusadapter/sharecontrol_adapter.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

static constexpr char kUserShareObjPath[] { "/com/deepin/filemanager/daemon/UserShareManager" };
static constexpr char kPolicyKitActionId[] { "com.deepin.filemanager.daemon.UserShareManager" };

ShareControlDBus::ShareControlDBus(QObject *parent)
    : QObject(parent), QDBusContext()
{
    QDBusConnection::systemBus().registerObject(kUserShareObjPath, this);
    adapter = new ShareControlAdapter(this);
}

ShareControlDBus::~ShareControlDBus()
{
    if (adapter)
        delete adapter;
    adapter = nullptr;
}

bool ShareControlDBus::CloseSmbShareByShareName(const QString &name, bool show)
{
    if (!show) {
        return true;
    }

    unsigned int suid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (!c.isConnected()) {
        qDebug() << "DBus connect failed";
        return false;
    }
    suid = c.interface()->serviceUid(message().service()).value();   //获取调用总线进程属主

    QString sharePath = "/var/lib/samba/usershares/";
    QString filePath = QString("%1%2").arg(sharePath).arg(name.toLower());   //文件名小写
    QFileInfo info(filePath);
    if ((suid != 0 && suid != info.ownerId())   //对比文件属主与调用总线进程属主;
        || info.isSymLink()   //禁止使用符合链接
        || !info.absoluteFilePath().startsWith(sharePath)) {   //禁止使用../等
        qInfo() << "invoker doesn't own the file: " << info.path();
        return false;
    }

    QProcess p;
    //取得所有连击的pid
    QString cmd = QString("smbcontrol smbd close-share %1").arg(name);
    qDebug() << "execute: " << cmd;
    p.start(cmd);
    bool ret = p.waitForFinished();

    qDebug() << "close smb share" << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool ShareControlDBus::SetUserSharePassword(const QString &name, const QString &passwd)
{
    if (!checkAuthentication()) {
        qInfo() << "cannot authenticate for user" << name << ", give up set password";
        return false;
    }

    qDebug() << name;   // << passward; // log password?
    QStringList args;
    args << "-a" << name << "-s";
    QProcess p;
    p.start("smbpasswd", args);
    p.write(passwd.toStdString().c_str());
    p.write("\n");
    p.write(passwd.toStdString().c_str());
    p.closeWriteChannel();
    bool ret = p.waitForFinished();
    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool ShareControlDBus::EnableSmbServices()
{
    // 创建链接文件之前已经提权了 这里就不需要再次判断权限了
    /*if (!checkAuthentication()) {
        qDebug() << "EnableSmbServices";
        return false;
    }*/

    QProcess sh;
    sh.start("ln -sf /lib/systemd/system/smbd.service /etc/systemd/system/multi-user.target.wants/smbd.service");
    auto ret = sh.waitForFinished();
    qInfo() << "enable smbd: " << ret;

    sh.start("ln -sf /lib/systemd/system/nmbd.service /etc/systemd/system/multi-user.target.wants/nmbd.service");
    ret &= sh.waitForFinished();
    qInfo() << "enable nmbd: " << ret;
    return ret;
}

bool ShareControlDBus::IsUserSharePasswordSet(const QString &username)
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

bool ShareControlDBus::checkAuthentication()
{
    if (!DAEMONPSHARECONTROL_NAMESPACE::PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionId, message().service())) {
        qInfo() << "Authentication failed !!";
        return false;
    }
    return true;
}
