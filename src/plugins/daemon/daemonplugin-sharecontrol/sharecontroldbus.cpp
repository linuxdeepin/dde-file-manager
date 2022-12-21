/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    QString filePath = QString("%1%2").arg(sharePath).arg(name.toLower());//文件名小写
    QFileInfo info(filePath);
    if ((suid != 0 && suid != info.ownerId())  //对比文件属主与调用总线进程属主;
            || info.isSymLink()      //禁止使用符合链接
            || !info.absoluteFilePath().startsWith(sharePath)) {   //禁止使用../等
        qDebug() << "invoker doesn't own the file: " << info.path();
        return false;
    }

    QProcess p;
    //取得所有连击的pid
    QString cmd = QString("smbcontrol smbd close-share %1").arg(name);
    qDebug() << "execute: " << cmd;
    p.start(cmd);
    bool ret = p.waitForFinished();

    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool ShareControlDBus::SetUserSharePassword(const QString &name, const QString &passwd)
{
    if (!checkAuthentication()) {
        qDebug() << "setUserSharePassword failed" << name;
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

bool ShareControlDBus::CreateShareLinkFile()
{
    // 创建链接文件之前已经提权了 这里就不需要再次判断权限了
    /*if (!checkAuthentication()) {
        qDebug() << "createShareLinkFile";
        return false;
    }*/

    QProcess sh;
    sh.start("ln -sf /lib/systemd/system/smbd.service /etc/systemd/system/multi-user.target.wants/smbd.service");
    auto ret = sh.waitForFinished();
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
