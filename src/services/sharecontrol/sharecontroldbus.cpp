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
#include <QTextStream>
#include <QRegularExpression>
#include <unistd.h>
#include <QDataStream>
#include <QByteArray>

static constexpr char kUserShareObjPath[] { "/org/deepin/Filemanager/UserShareManager" };
static constexpr char kPolicyKitActionId[] { "org.deepin.Filemanager.UserShareManager" };

namespace service_sharecontrol {
DFM_LOG_REISGER_CATEGORY(SERVICESHARECONTROL_NAMESPACE)
}

SERVICESHARECONTROL_USE_NAMESPACE

ShareControlDBus::ShareControlDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext()
{
    adapter = new UserShareManagerAdaptor(this);
    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name)).registerObject(kUserShareObjPath, this, QDBusConnection::ExportAdaptors);
}

ShareControlDBus::~ShareControlDBus()
{
}

bool ShareControlDBus::CloseSmbShareByShareName(const QString &name, bool show)
{
    if (!show) {
        return true;
    }

    if (!checkAuthentication()) {
        fmInfo() << "cannot close smb for" << name;
        return false;
    }

    unsigned int suid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (!c.isConnected()) {
        fmDebug() << "DBus connect failed";
        return false;
    }
    suid = c.interface()->serviceUid(message().service()).value();   // 获取调用总线进程属主

    QString sharePath = "/var/lib/samba/usershares/";
    QString filePath = QString("%1%2").arg(sharePath).arg(name.toLower());   // 文件名小写
    QFileInfo info(filePath);
    if ((suid != 0 && suid != info.ownerId())   // 对比文件属主与调用总线进程属主;
        || info.isSymLink()   // 禁止使用符合链接
        || !info.absoluteFilePath().startsWith(sharePath)) {   // 禁止使用../等
        fmInfo() << "invoker doesn't own the file: " << info.path();
        return false;
    }

    QProcess p;
    // 取得所有连击的pid
    QString program = "smbcontrol";
    QStringList arguments = { "smbd", "close-share", name };
    p.start(program, arguments);
    bool ret = p.waitForFinished();

    fmDebug() << "close smb share" << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool ShareControlDBus::SetUserSharePassword(const QDBusUnixFileDescriptor &credentialsFd)
{
    if (!checkAuthentication()) {
        fmInfo() << "Authentication failed, give up set password";
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
    fmDebug() << "smbpasswd result:" << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();

    return r && (p.exitCode() == 0);
}

bool ShareControlDBus::EnableSmbServices()
{
    if (!checkAuthentication()) {
        fmDebug() << "EnableSmbServices";
        return false;
    }

    QProcess sh;
    sh.start("ln", { "-sf", "/lib/systemd/system/smbd.service", "/etc/systemd/system/multi-user.target.wants/smbd.service" });
    auto ret = sh.waitForFinished();
    fmInfo() << "enable smbd: " << ret;

    sh.start("ln", { "-sf", "/lib/systemd/system/nmbd.service", "/etc/systemd/system/multi-user.target.wants/nmbd.service" });
    ret &= sh.waitForFinished();
    fmInfo() << "enable nmbd: " << ret;
    return ret;
}

bool ShareControlDBus::IsUserSharePasswordSet(const QString &username)
{
    QProcess p;
    p.start("pdbedit", { "-L" });
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
    if (!SERVICESHARECONTROL_NAMESPACE::PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionId, message().service())) {
        fmInfo() << "Authentication failed !!";
        return false;
    }
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
