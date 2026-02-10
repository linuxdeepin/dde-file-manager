// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fstabhelper.h"

#include <QFile>
#include <QRegularExpression>

static constexpr char kFstabPath[] { "/etc/fstab" };

// fstab file fields index.
enum FieldIndex {
    FileSystem,
    MountPoint,
    Type,
    Options,
    Dump,
    Pass
};

FILE_ENCRYPT_USE_NS

bool fstab_helper::setFstabPassno(const QString &devUUID, int passno)
{
    qInfo() << "[fstab_helper::setFstabPassno] Setting fstab passno for device UUID:" << devUUID << "passno:" << passno;
    
    QFile f(kFstabPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "[fstab_helper::setFstabPassno] Failed to open fstab file for reading:" << kFstabPath;
        return false;
    }

    auto contents = f.readAll().split('\n');
    f.close();

    bool needUpdate { false };
    for (auto &item : contents) {
        if (item.startsWith("#")) continue;

        auto fields = QString(item).split(QRegularExpression(R"( |\t)"), Qt::SkipEmptyParts);
        if (fields.count() < 6) continue;

        auto source = fields.at(FileSystem);
        auto pass = fields.at(Pass);
        if (source == devUUID
            && pass.toInt() != passno) {
            needUpdate = true;

            fields[Pass] = QString::number(passno);
            item = fields.join("\t").toLocal8Bit();
            qInfo() << "[fstab_helper::setFstabPassno] Updated fstab entry for device:" << devUUID << "old passno:" << pass << "new passno:" << passno;
            break;
        }
    }

    if (!needUpdate) {
        qInfo() << "[fstab_helper::setFstabPassno] No update needed for device:" << devUUID << "passno already set to:" << passno;
        return true;
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[fstab_helper::setFstabPassno] Failed to open fstab file for writing:" << kFstabPath;
        return false;
    }

    f.write(contents.join('\n'));
    f.flush();
    f.close();

    qInfo() << "[fstab_helper::setFstabPassno] Fstab file updated successfully for device:" << devUUID;
    return true;
}

bool fstab_helper::setFstabTimeout(const QString &devPath, const QString &devUUID)
{
    qInfo() << "[fstab_helper::setFstabTimeout] Setting fstab timeout for device - path:" << devPath << "UUID:" << devUUID;
    
    const QString kArg = "x-systemd.device-timeout=0";
    QFile f(kFstabPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "[fstab_helper::setFstabTimeout] Failed to open fstab file for reading:" << kFstabPath;
        return false;
    }

    auto contents = f.readAll().split('\n');
    f.close();

    bool needUpdate { false };
    for (auto &item : contents) {
        if (item.startsWith("#")) continue;

        auto fields = QString(item).split(QRegularExpression(R"( |\t)"), Qt::SkipEmptyParts);
        if (fields.count() < 6) continue;

        auto source = fields.at(FileSystem);
        auto options = fields.at(Options);
        if ((source == devPath || source == devUUID)
            && !options.contains(kArg)) {
            needUpdate = true;

            fields[Options] += { "," + kArg };
            item = fields.join("\t").toLocal8Bit();
            qInfo() << "[fstab_helper::setFstabTimeout] Added timeout option to fstab entry for device:" << source << "new options:" << fields[Options];
            break;
        }
    }

    if (!needUpdate) {
        qInfo() << "[fstab_helper::setFstabTimeout] No update needed, timeout option already exists for device - path:" << devPath << "UUID:" << devUUID;
        return true;
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[fstab_helper::setFstabTimeout] Failed to open fstab file for writing:" << kFstabPath;
        return false;
    }

    f.write(contents.join('\n'));
    f.flush();
    f.close();

    qInfo() << "[fstab_helper::setFstabTimeout] Fstab file updated successfully with timeout option for device - path:" << devPath << "UUID:" << devUUID;
    return true;
}
