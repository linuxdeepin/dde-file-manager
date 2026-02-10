// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QTextStream>
#include <QRegularExpression>
#include <unistd.h>
#include <QDataStream>
#include <QByteArray>

static constexpr char kUserShareObjPath[] { "/org/deepin/Filemanager/UserShareManager" };
static constexpr char kPolicyKitActionId[] { "org.deepin.Filemanager.UserShareManager" };

namespace service_sharecontrol {
DFM_LOG_REGISTER_CATEGORY(SERVICESHARECONTROL_NAMESPACE)
}

SERVICESHARECONTROL_USE_NAMESPACE

using ServiceCommon::PolicyKitHelper;

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

bool ShareControlDBus::SetUserSharePassword(const QDBusUnixFileDescriptor &credentialsFd)
{
    fmInfo() << "[ShareControlDBus::SetUserSharePassword] Request to set password";

    if (!checkAuthentication()) {
        fmWarning() << "[ShareControlDBus::SetUserSharePassword] Authentication failed";
        return false;
    }

    if (!credentialsFd.isValid()) {
        fmInfo() << "Invalid file descriptor provided";
        return false;
    }

    // Read credentials from pipe file descriptor
    int fd = credentialsFd.fileDescriptor();
    if (fd < 0) {
        fmInfo() << "Invalid file descriptor value:" << fd;
        return false;
    }

    // Read all data from pipe into buffer
    QByteArray buffer;
    char readBuffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, readBuffer, sizeof(readBuffer))) > 0) {
        buffer.append(readBuffer, bytesRead);
    }

    if (buffer.isEmpty()) {
        fmInfo() << "No data received from pipe";
        return false;
    }

    // Parse credentials using QDataStream (matching client-side serialization)
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    QString username, password;
    stream >> username >> password;

    if (stream.status() != QDataStream::Ok) {
        fmInfo() << "Failed to parse credentials from pipe data, stream status:" << stream.status();
        return false;
    }

    // Validate username to prevent command injection
    if (!isValidUsername(username)) {
        fmInfo() << "Invalid username provided:" << username;
        return false;
    }

    // Decrypt password
    QString passwdDec = dfmbase::FileUtils::decryptString(password);

    // Set the password using smbpasswd
    QStringList args;
    args << "-a" << username << "-s";
    QProcess p;
    fmInfo() << "[ShareControlDBus::SetUserSharePassword] Executing smbpasswd command";
    p.start("smbpasswd", args);
    if (!p.waitForStarted()) {
        fmInfo() << "Failed to start smbpasswd process";
        return false;
    }

    p.write(passwdDec.toUtf8());
    p.write("\n");
    p.write(passwdDec.toUtf8());
    p.closeWriteChannel();

    bool r = p.waitForFinished();

    if (r) {
        fmInfo() << "[ShareControlDBus::SetUserSharePassword] Successfully set password";
    } else {
        fmCritical() << "[ShareControlDBus::SetUserSharePassword] Failed to set password";
    }

    fmDebug() << "[ShareControlDBus::SetUserSharePassword] smbpasswd output. stdout:"
              << p.readAllStandardOutput() << "stderr:" << p.readAllStandardError();
    return r;
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
    if (!PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionId, message().service())) {
        fmWarning() << "[ShareControlDBus::checkAuthentication] Authentication failed for action ID:" << kPolicyKitActionId << "service:" << message().service();
        return false;
    }
    fmInfo() << "[ShareControlDBus::checkAuthentication] Authentication successful for action ID:" << kPolicyKitActionId;
    return true;
}

bool ShareControlDBus::isValidUsername(const QString &username) const
{
    if (username.isEmpty() || username.length() > 32) {
        return false;
    }

    // Username should only contain alphanumeric characters, underscore, and hyphen
    // Should not start with hyphen to prevent command line parameter injection
    static QRegularExpression kRegex("^[a-zA-Z0-9_][a-zA-Z0-9_-]*$");
    if (!kRegex.match(username).hasMatch()) {
        return false;
    }

    // Additional check: username should not contain sequences that could be interpreted as options
    if (username.startsWith("-") || username.contains("--")) {
        return false;
    }

    return true;
}
