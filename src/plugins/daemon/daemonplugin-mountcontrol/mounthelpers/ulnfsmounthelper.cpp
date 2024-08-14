// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ulnfsmounthelper.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>

#include <libmount/libmount.h>

DAEMONPMOUNTCONTROL_USE_NAMESPACE

QVariantMap UlnfsMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)   // not used yet and might be used in the future.
    using namespace MountReturnField;

    static constexpr char kUlnfs[] { "ulnfs" };

    // 1. check if ulnfs/dlnfs  at `path` is already exist.
    if (checkLnfsExist(path))
        return { { kResult, true },
                 { kErrorCode, -kLnMountMounted },
                 { kErrorMessage, QString("lnfs is already mounted at %1").arg(path) } };

    // 2. check `ulnfs` is loaded.
    if (!isModuleLoaded(kUlnfs) && !loadKernelModule(kUlnfs))
        return { { kResult, false },
                 { kErrorCode, -kLnFsProcessNotExists },
                 { kErrorMessage, QString("ulnfs is not loaded.") } };

    // 3. mount ulnfs on `path`
    auto aPath = path.toLocal8Bit().constData();
    int ret = ::mount(aPath, aPath, kUlnfs, 0, "");

    fmInfo() << "ulnfs: mount result: " << ret << aPath;
    if (ret == 0) {
        return { { kMountPoint, aPath }, { kResult, true }, { kErrorCode, 0 } };
    } else {
        int errNum = errno;
        QString errMsg = strerror(errNum);
        return { { kResult, false },
                 { kErrorMessage, errMsg },
                 { kErrorCode, parseErrorCodeByMsg(ret) } };
    }
}

QVariantMap UlnfsMountHelper::unmount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)
    using namespace MountReturnField;

    // 1. check if ulnfs is already mounted at `path`
    if (!checkLnfsExist(path)) {
        fmDebug() << "dlnfs: is not mounted at" << path;
        return { { kResult, true },
                 { kErrorCode, -kMountNotExist },
                 { kErrorMessage, QString("dlnfs is not mounted at %1").arg(path) } };
    }

    // 2. do unmount ulnfs

    int ret = ::umount(path.toStdString().c_str());
    int err = errno;
    QString errMsg = strerror(errno);
    if (ret != 0)
        fmWarning() << "unmount failed: " << path << err << errMsg;

    return { { kResult, ret == 0 }, { kErrorCode, err }, { kErrorMessage, errMsg } };
}

bool UlnfsMountHelper::isModuleLoaded(const QString &moduleName)
{
    QProcess process;
    process.start("lsmod");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    foreach (const QString &line, lines) {
        if (line.startsWith(moduleName)) {
            return true;
        }
    }
    return false;
}

bool UlnfsMountHelper::loadKernelModule(const QString &moduleName)
{
    QProcess modprobeProcess;

    modprobeProcess.start("modprobe", QStringList() << moduleName);

    if (!modprobeProcess.waitForFinished()) {
        qCritical("Failed to load module: %s", qPrintable(modprobeProcess.errorString()));
        return false;
    }

    QByteArray output = modprobeProcess.readAllStandardOutput();
    qDebug() << moduleName << "Module loaded successfully.";
    return true;
}

bool UlnfsMountHelper::checkLnfsExist(const QString &path)
{
    QFile mountsFile("/proc/mounts");
    if (!mountsFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open /proc/mounts.";
        return false;
    }

    QByteArray content = mountsFile.readAll();
    QString fileContent(content);

    QStringList lines = fileContent.split('\n');
    for (const QString &line : lines) {
        QStringList parts = line.split(' ');

        if (parts.size() >= 3) {
            QString mountPoint = parts[1];
            QString fsType = parts[2];

            if (mountPoint == path && fsType == "ulnfs") {
                return true;
            }
        }
    }

    return false;
}

int UlnfsMountHelper::parseErrorCodeByMsg(int res)
{
    if (res == 0)
        return 0;
    return -kUnhandledError;
}
