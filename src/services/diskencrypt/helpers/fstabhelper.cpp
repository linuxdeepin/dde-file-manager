// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    QFile f(kFstabPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "cannot open file!" << kFstabPath;
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
            break;
        }
    }

    if (!needUpdate)
        return true;

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "cannot open file for write!" << kFstabPath;
        return false;
    }

    f.write(contents.join('\n'));
    f.flush();
    f.close();

    return true;
}

bool fstab_helper::setFstabTimeout(const QString &devPath, const QString &devUUID)
{
    const QString kArg = "x-systemd.device-timeout=0";
    QFile f(kFstabPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "cannot open file!" << kFstabPath;
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
            break;
        }
    }

    if (!needUpdate)
        return true;

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "cannot open file for write!" << kFstabPath;
        return false;
    }

    f.write(contents.join('\n'));
    f.flush();
    f.close();

    return true;
}
