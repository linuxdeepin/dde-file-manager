/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "usersharemanager.h"
#include "dbusservice/dbusadaptor/usershare_adaptor.h"
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
    bool ret = QProcess::startDetached("/usr/sbin/groupadd", args);
    qDebug() << "groupadd" << groupName << ret;
    return ret;
}

bool UserShareManager::setUserSharePassword(const QString &username, const QString &passward)
{
    if (!checkAuthentication()) {
        qDebug() << "setUserSharePassword failed" <<  username;
        return false;
    }

    qDebug() << username;// << passward; // log password?
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
    sh.start("ln -sf /lib/systemd/system/smbd.service /etc/systemd/system/multi-user.target.wants/smbd.service");
    auto ret = sh.waitForFinished();
    return ret;
}
