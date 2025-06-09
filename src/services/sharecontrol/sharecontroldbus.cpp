// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecontroldbus.h"
#include "polkit/policykithelper.h"
#include "sharecontroladaptor.h"
#include "service_sharecontrol_global.h"

#include <dfm-base/utils/fileutils.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

static constexpr char kUserShareObjPath[] { "/org/deepin/Filemanager/UserShareManager" };
static constexpr char kPolicyKitActionId[] { "org.deepin.Filemanager.UserShareManager" };

namespace service_sharecontrol {
DFM_LOG_REISGER_CATEGORY(SERVICESHARECONTROL_NAMESPACE)
}

SERVICESHARECONTROL_USE_NAMESPACE

ShareControlDBus::ShareControlDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext()
{
    fmInfo() << "[ShareControlDBus] Initializing share control service with name:" << name;
    adapter = new UserShareManagerAdaptor(this);
    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name)).registerObject(kUserShareObjPath, this, QDBusConnection::ExportAdaptors);
    fmInfo() << "[ShareControlDBus] Share control service registered successfully";
}

ShareControlDBus::~ShareControlDBus()
{
    fmInfo() << "[ShareControlDBus] Share control service destroyed";
}

bool ShareControlDBus::CloseSmbShareByShareName(const QString &name, bool show)
{
    fmInfo() << "[ShareControlDBus::CloseSmbShareByShareName] Request to close SMB share:" << name << "show:" << show;
    
    if (!show) {
        fmInfo() << "[ShareControlDBus::CloseSmbShareByShareName] Show parameter is false, operation skipped for share:" << name;
        return true;
    }

    if (!checkAuthentication()) {
        fmWarning() << "[ShareControlDBus::CloseSmbShareByShareName] Authentication failed for closing SMB share:" << name;
        return false;
    }

    unsigned int suid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (!c.isConnected()) {
        fmCritical() << "[ShareControlDBus::CloseSmbShareByShareName] Failed to connect to D-Bus for share:" << name;
        return false;
    }
    suid = c.interface()->serviceUid(message().service()).value();   // 获取调用总线进程属主

    QString sharePath = "/var/lib/samba/usershares/";
    QString filePath = QString("%1%2").arg(sharePath).arg(name.toLower());   // 文件名小写
    QFileInfo info(filePath);
    
    fmInfo() << "[ShareControlDBus::CloseSmbShareByShareName] Checking share file permissions - path:" << filePath << "caller UID:" << suid << "file owner UID:" << info.ownerId();
    
    if ((suid != 0 && suid != info.ownerId())   // 对比文件属主与调用总线进程属主;
        || info.isSymLink()   // 禁止使用符合链接
        || !info.absoluteFilePath().startsWith(sharePath)) {   // 禁止使用../等
        fmWarning() << "[ShareControlDBus::CloseSmbShareByShareName] Permission denied - invoker does not own the share file:" << info.absoluteFilePath() << "or security violation detected";
        return false;
    }

    QProcess p;
    // 取得所有连击的pid
    QString program = "smbcontrol";
    QStringList arguments = { "smbd", "close-share", name };
    fmInfo() << "[ShareControlDBus::CloseSmbShareByShareName] Executing smbcontrol command for share:" << name;
    p.start(program, arguments);
    bool ret = p.waitForFinished();

    if (ret) {
        fmInfo() << "[ShareControlDBus::CloseSmbShareByShareName] Successfully closed SMB share:" << name;
    } else {
        fmCritical() << "[ShareControlDBus::CloseSmbShareByShareName] Failed to close SMB share:" << name << "process output:" << p.readAll() << "stderr:" << p.readAllStandardError();
    }
    
    fmDebug() << "[ShareControlDBus::CloseSmbShareByShareName] Command output for share:" << name << "stdout:" << p.readAllStandardOutput() << "stderr:" << p.readAllStandardError();
    return ret;
}

bool ShareControlDBus::SetUserSharePassword(const QString &name, const QString &passwd)
{
    fmInfo() << "[ShareControlDBus::SetUserSharePassword] Request to set password for user:" << name;
    
    if (!checkAuthentication()) {
        fmWarning() << "[ShareControlDBus::SetUserSharePassword] Authentication failed for user:" << name;
        return false;
    }

    QString passwdDec = dfmbase::FileUtils::decryptString(passwd);

    QStringList args;
    args << "-a" << name << "-s";
    QProcess p;
    fmInfo() << "[ShareControlDBus::SetUserSharePassword] Executing smbpasswd command for user:" << name;
    p.start("smbpasswd", args);
    p.write(passwdDec.toStdString().c_str());
    p.write("\n");
    p.write(passwdDec.toStdString().c_str());
    p.closeWriteChannel();
    bool r = p.waitForFinished();
    
    if (r) {
        fmInfo() << "[ShareControlDBus::SetUserSharePassword] Successfully set password for user:" << name;
    } else {
        fmCritical() << "[ShareControlDBus::SetUserSharePassword] Failed to set password for user:" << name;
    }
    
    fmDebug() << "[ShareControlDBus::SetUserSharePassword] smbpasswd output for user:" << name << "stdout:" << p.readAllStandardOutput() << "stderr:" << p.readAllStandardError();
    return r;
}

bool ShareControlDBus::EnableSmbServices()
{
    fmInfo() << "[ShareControlDBus::EnableSmbServices] Request to enable SMB services";
    
    if (!checkAuthentication()) {
        fmWarning() << "[ShareControlDBus::EnableSmbServices] Authentication failed for enabling SMB services";
        return false;
    }

    QProcess sh;
    fmInfo() << "[ShareControlDBus::EnableSmbServices] Enabling smbd service";
    sh.start("ln", { "-sf", "/lib/systemd/system/smbd.service", "/etc/systemd/system/multi-user.target.wants/smbd.service" });
    auto ret = sh.waitForFinished();
    if (ret) {
        fmInfo() << "[ShareControlDBus::EnableSmbServices] Successfully enabled smbd service";
    } else {
        fmCritical() << "[ShareControlDBus::EnableSmbServices] Failed to enable smbd service";
    }

    fmInfo() << "[ShareControlDBus::EnableSmbServices] Enabling nmbd service";
    sh.start("ln", { "-sf", "/lib/systemd/system/nmbd.service", "/etc/systemd/system/multi-user.target.wants/nmbd.service" });
    ret &= sh.waitForFinished();
    if (ret) {
        fmInfo() << "[ShareControlDBus::EnableSmbServices] Successfully enabled nmbd service";
    } else {
        fmCritical() << "[ShareControlDBus::EnableSmbServices] Failed to enable nmbd service";
    }
    
    fmInfo() << "[ShareControlDBus::EnableSmbServices] SMB services enablement completed with result:" << ret;
    return ret;
}

bool ShareControlDBus::IsUserSharePasswordSet(const QString &username)
{
    fmInfo() << "[ShareControlDBus::IsUserSharePasswordSet] Checking if password is set for user:" << username;
    
    QProcess p;
    p.start("pdbedit", { "-L" });
    auto ret = p.waitForFinished();
    
    if (!ret) {
        fmCritical() << "[ShareControlDBus::IsUserSharePasswordSet] Failed to execute pdbedit command for user:" << username;
        return false;
    }
    
    QStringList resultLines = QString::fromUtf8(p.readAllStandardOutput()).split('\n');
    bool isPasswordSet = false;
    foreach (const QString &line, resultLines) {
        if (line.startsWith(username + ":")) {
            isPasswordSet = true;
            break;
        }
    }

    fmInfo() << "[ShareControlDBus::IsUserSharePasswordSet] Password check result for user:" << username << "is set:" << isPasswordSet;
    return ret && isPasswordSet;
}

bool ShareControlDBus::checkAuthentication()
{
    if (!SERVICESHARECONTROL_NAMESPACE::PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionId, message().service())) {
        fmWarning() << "[ShareControlDBus::checkAuthentication] Authentication failed for action ID:" << kPolicyKitActionId << "service:" << message().service();
        return false;
    }
    fmInfo() << "[ShareControlDBus::checkAuthentication] Authentication successful for action ID:" << kPolicyKitActionId;
    return true;
}
