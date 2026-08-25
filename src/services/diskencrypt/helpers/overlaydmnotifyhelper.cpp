// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "overlaydmnotifyhelper.h"
#include "globaltypesdefine.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDateTime>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

FILE_ENCRYPT_USE_NS

OverlayDMNotifyHelper *OverlayDMNotifyHelper::instance()
{
    static OverlayDMNotifyHelper ins;
    return &ins;
}

OverlayDMNotifyHelper::OverlayDMNotifyHelper(QObject *parent)
    : QObject(parent)
{
}

OverlayDMNotifyHelper::ActiveUser OverlayDMNotifyHelper::findActiveGraphicalUser()
{
    ActiveUser user;

    QProcess loginctl;
    loginctl.start("loginctl", { "list-sessions", "--no-legend" });
    if (!loginctl.waitForFinished(5000)) {
        qWarning() << "[OverlayDMNotifyHelper::findActiveGraphicalUser] loginctl list-sessions timed out";
        loginctl.kill();
        return user;
    }

    const QString output = QString::fromUtf8(loginctl.readAllStandardOutput());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        const QStringList parts = line.simplified().split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 4)
            continue;

        const QString &sessionId = parts[0];

        QProcess show;
        show.start("loginctl", { "show-session", sessionId, "-p", "Active", "-p", "Type", "-p", "Name", "-p", "User" });
        if (!show.waitForFinished(5000)) {
            qWarning() << "[OverlayDMNotifyHelper::findActiveGraphicalUser] show-session timed out for session:" << sessionId;
            show.kill();
            continue;
        }

        const QString showOutput = QString::fromUtf8(show.readAllStandardOutput());
        const QStringList propLines = showOutput.split('\n', Qt::SkipEmptyParts);

        QHash<QString, QString> props;
        for (const QString &propLine : propLines) {
            const int eq = propLine.indexOf('=');
            if (eq < 0)
                continue;
            const QString key = propLine.left(eq).simplified();
            const QString val = propLine.mid(eq + 1).simplified();
            props[key] = val;
        }

        if (!props.contains("Active") || !props.contains("Type")
            || !props.contains("Name") || !props.contains("User"))
            continue;

        const QString &active = props.value("Active");
        const QString &type = props.value("Type");
        const QString &username = props.value("Name");
        const QString &uidStr = props.value("User");

        if (active != "yes")
            continue;
        if (type != "wayland" && type != "x11")
            continue;

        bool ok = false;
        uint uid = uidStr.toUInt(&ok);
        if (!ok || uid == 0)
            continue;

        user.uid = uid;
        user.username = username;
        user.valid = true;

        qInfo() << "[OverlayDMNotifyHelper::findActiveGraphicalUser] Found active graphical user:" << username << "UID:" << uid;
        return user;
    }

    qInfo() << "[OverlayDMNotifyHelper::findActiveGraphicalUser] No active graphical session found";
    return user;
}

bool OverlayDMNotifyHelper::isFileManagerRunning(uint uid)
{
    QProcess pgrep;
    pgrep.start("pgrep", { "-u", QString::number(uid), "-x", "dde-file-manager" });
    if (!pgrep.waitForFinished(5000)) {
        qWarning() << "[OverlayDMNotifyHelper::isFileManagerRunning] pgrep timed out";
        pgrep.kill();
        return false;
    }

    bool running = (pgrep.exitCode() == 0);
    qInfo() << "[OverlayDMNotifyHelper::isFileManagerRunning] UID:" << uid << "file manager running:" << running;
    return running;
}

void OverlayDMNotifyHelper::launchFileManager(uint uid, const QString &username)
{
    qInfo() << "[OverlayDMNotifyHelper::launchFileManager] Launching file manager for user:" << username << "UID:" << uid;

    const QString busAddr = QString("unix:path=/run/user/%1/bus").arg(uid);

    QProcess process;
    process.setProgram("runuser");
    process.setArguments({ "-u", username, "--",
                           "env", QString("DBUS_SESSION_BUS_ADDRESS=%1").arg(busAddr),
                           "/usr/libexec/dde-file-manager", "-d" });

    qint64 pid = 0;
    if (process.startDetached(&pid)) {
        qInfo() << "[OverlayDMNotifyHelper::launchFileManager] File manager launched, PID:" << pid;
    } else {
        qWarning() << "[OverlayDMNotifyHelper::launchFileManager] Failed to launch file manager:" << process.errorString();
    }
}

void OverlayDMNotifyHelper::writePendingFile(bool enabled, int result, const ActiveUser &user)
{
    QDir dir(disk_encrypt::kOverlayDMNotifyDir);
    if (!dir.exists()) {
        if (!dir.mkpath(disk_encrypt::kOverlayDMNotifyDir)) {
            qWarning() << "[OverlayDMNotifyHelper::writePendingFile] Failed to create directory:" << disk_encrypt::kOverlayDMNotifyDir;
            return;
        }
        chmod(disk_encrypt::kOverlayDMNotifyDir, 0755);
        if (user.valid) {
            chown(disk_encrypt::kOverlayDMNotifyDir, user.uid, user.uid);
        }
    }

    // Remove stale pending file so the plugin won't re-process old notifications.
    unlink(disk_encrypt::kOverlayDMNotifyFile);

    // O_NOFOLLOW prevents symlink attacks: if an attacker places a symlink at the
    // notify path, open() fails with ELOOP instead of following it to a target file.
    int fd = open(disk_encrypt::kOverlayDMNotifyFile,
                  O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644);
    if (fd < 0) {
        qWarning() << "[OverlayDMNotifyHelper::writePendingFile] Failed to open file:" << disk_encrypt::kOverlayDMNotifyFile
                   << "errno:" << errno;
        return;
    }

    QJsonObject json;
    json["enabled"] = enabled;
    json["result"] = result;
    json["timestamp"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());
    if (user.valid)
        json["uid"] = static_cast<qint64>(user.uid);

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    if (write(fd, data.constData(), data.size()) < 0) {
        qWarning() << "[OverlayDMNotifyHelper::writePendingFile] Failed to write file, errno:" << errno;
        close(fd);
        return;
    }

    // Operate on the fd to avoid TOCTOU races between open() and chmod()/chown().
    fchmod(fd, 0644);
    if (user.valid) {
        fchown(fd, user.uid, user.uid);
    }

    close(fd);

    qInfo() << "[OverlayDMNotifyHelper::writePendingFile] Pending notification written:" << disk_encrypt::kOverlayDMNotifyFile
            << "enabled:" << enabled << "result:" << result;
}

void OverlayDMNotifyHelper::ensureNotificationDelivery(bool enabled, int result)
{
    qInfo() << "[OverlayDMNotifyHelper::ensureNotificationDelivery] Ensuring notification delivery, enabled:" << enabled << "result:" << result;

    ActiveUser user = findActiveGraphicalUser();

    writePendingFile(enabled, result, user);

    if (!user.valid) {
        qInfo() << "[OverlayDMNotifyHelper::ensureNotificationDelivery] No active graphical user, pending file written for later delivery";
        return;
    }

    if (!isFileManagerRunning(user.uid)) {
        launchFileManager(user.uid, user.username);
    } else {
        qInfo() << "[OverlayDMNotifyHelper::ensureNotificationDelivery] File manager already running, signal will deliver notification";
    }
}
