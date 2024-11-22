// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILTININTERFACE_H
#define BUILTININTERFACE_H

#include <QFile>
#include <QStandardPaths>
#include <QLibrary>
#include <QDir>
#include <QDebug>

#include <unistd.h>

namespace dfm_upgrade {

#define GetUpgradeLibraryPath(path)                                                     \
    {                                                                                   \
        QString libPath(qApp->applicationDirPath() + "/../../tools/libdfm-upgrade.so"); \
        if (!QFile::exists(libPath)) {                                                  \
            libPath = QString(DFM_TOOLS_DIR) + "/libdfm-upgrade.so";                    \
            qInfo() << QString("library does not exist, use : %1").arg(libPath);        \
        }                                                                               \
        path = libPath;                                                                 \
    }

inline constexpr char kUpgradeFlag[] = "dfm-upgraded.lock";
inline constexpr char kArgDesktop[] = "Desktop";
inline constexpr char kArgFileManger[] = "FileManager";

typedef int (*UpgradeFunc)(const QMap<QString, QString> &);

inline QString upgradeConfigDir()
{
    return QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager";
}

inline bool isNeedUpgrade()
{
    const QString dirPath = upgradeConfigDir();
    if (QFile::exists(dirPath + "/" + kUpgradeFlag)) {
        QFileInfo dir(dirPath);
        if (!dir.isWritable()) {
            qCritical() << "give up upgrading:the config dir is not writable" << dirPath;
            return false;
        }
        return true;
    }
    return false;
}

inline int tryUpgrade(const QString &libPath, const QMap<QString, QString> &args)
{
    // load library
    QLibrary lib(libPath);
    if (!lib.load()) {
        qCritical() << "fail to load upgrade library:" << lib.errorString();
        return 1;
    }

    auto func = reinterpret_cast<UpgradeFunc>(lib.resolve("dfm_tools_upgrade_doUpgrade"));
    if (!func) {
        qCritical() << "no upgrade function in :" << lib.fileName();
        return 1;
    }

    int ret = func(args);
    lib.unload();

    return ret;
}

}

#endif   // BUILTININTERFACE_H
