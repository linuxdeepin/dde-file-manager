// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesystemhelper.h"
#include "blockdevhelper.h"

#include <dfm-base/utils/finallyutil.h>

#include <QFile>
#include <QDir>

#include <sys/stat.h>
#include <fstab.h>
#include <unistd.h>
#include <sys/mount.h>

FILE_ENCRYPT_USE_NS

bool filesystem_helper::shrinkFileSystem_ext(const QString &device)
{
    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Starting filesystem shrink operation for device:" << device;
    
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Running filesystem check command:" << cmd;
    
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qCritical() << "[filesystem_helper::shrinkFileSystem_ext] Filesystem check failed for device:" << device << "error code:" << ret;
        return false;
    }
    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Filesystem check completed successfully for device:" << device;

    auto size = blockdev_helper::devDeviceSize(device) / 1024 / 1024;
    auto targetSize = size - 32;
    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Device will be resized from" << size << "MB to" << targetSize << "MB for device:" << device;
    
    cmd = QString("resize2fs %1 %2M").arg(device).arg(targetSize);
    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Running resize command:" << cmd;
    
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qCritical() << "[filesystem_helper::shrinkFileSystem_ext] Filesystem resize failed for device:" << device << "error code:" << ret;
        return false;
    }

    qInfo() << "[filesystem_helper::shrinkFileSystem_ext] Filesystem shrink completed successfully for device:" << device;
    return true;
}

bool filesystem_helper::expandFileSystem_ext(const QString &device)
{
    qInfo() << "[filesystem_helper::expandFileSystem_ext] Starting filesystem expand operation for device:" << device;
    
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    qInfo() << "[filesystem_helper::expandFileSystem_ext] Running filesystem check command:" << cmd;
    
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qCritical() << "[filesystem_helper::expandFileSystem_ext] Filesystem check failed for device:" << device << "error code:" << ret;
        return false;
    }
    qInfo() << "[filesystem_helper::expandFileSystem_ext] Filesystem check completed successfully for device:" << device;

    cmd = QString("resize2fs %1").arg(device);
    qInfo() << "[filesystem_helper::expandFileSystem_ext] Running resize command:" << cmd;
    
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qCritical() << "[filesystem_helper::expandFileSystem_ext] Filesystem resize failed for device:" << device << "error code:" << ret;
        return false;
    }

    qInfo() << "[filesystem_helper::expandFileSystem_ext] Filesystem expand completed successfully for device:" << device;
    return true;
}

bool filesystem_helper::moveFsForward(const QString &dev)
{
    qInfo() << "[filesystem_helper::moveFsForward] Starting filesystem forward move operation for device:" << dev;
    
    static const quint64 kStepSize = 16 * 1024 * 1024;   // luks header size.
    char *buf = new char[kStepSize];
    QFile logFile, devFile;
    dfmbase::FinallyUtil atFinish([&] {
        if (logFile.isOpen()) logFile.close();
        if (devFile.isOpen()) devFile.close();
        if (buf) delete[] buf;
    });

    devFile.setFileName(dev);
    logFile.setFileName(QString(disk_encrypt::kUSecConfigDir) + "/dfm_mv_fs_" + dev.mid(5));
    
    if (!logFile.exists()) {
        qInfo() << "[filesystem_helper::moveFsForward] Creating new log file for operation:" << logFile.fileName();
        if (!logFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            qCritical() << "[filesystem_helper::moveFsForward] Failed to create filesystem move log file for device:" << dev;
            return false;
        }
        logFile.close();
    } else {
        qInfo() << "[filesystem_helper::moveFsForward] Using existing log file for operation:" << logFile.fileName();
    }

    if (!logFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)
        || !devFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        qCritical() << "[filesystem_helper::moveFsForward] Failed to open log file or device file for device:" << dev;
        return false;
    }

    auto size = blockdev_helper::devDeviceSize(dev);
    if (size == 0) {
        qCritical() << "[filesystem_helper::moveFsForward] Failed to obtain device size for device:" << dev;
        return false;
    }
    qInfo() << "[filesystem_helper::moveFsForward] Device size:" << size << "bytes, step size:" << kStepSize << "bytes for device:" << dev;

    auto count = size / kStepSize;
    if (size % kStepSize == 0) count -= 1;

    auto log = logFile.readAll().split(',');
    auto lastIdx = (log.count() > 0) ? log.last().toULongLong() : 0;   // breakpoint

    qInfo() << "[filesystem_helper::moveFsForward] Resuming from step:" << lastIdx << "total steps:" << count << "for device:" << dev;

    for (auto curIdx = lastIdx + 1;
         curIdx <= count;
         ++curIdx, ++lastIdx) {
        if (curIdx * kStepSize >= size) {
            qWarning() << "[filesystem_helper::moveFsForward] Seek position exceeds device size, stopping operation for device:" << dev;
            break;
        }

        // seek current move position
        if (!devFile.seek(curIdx * kStepSize)) {
            qCritical() << "[filesystem_helper::moveFsForward] Failed to seek to position:" << (curIdx * kStepSize) << "for device:" << dev;
            return false;
        }

        // read next step
        memset(buf, 0, kStepSize);
        quint64 readed = devFile.read(buf, kStepSize);

        // write to previous step position.
        if (!devFile.seek(lastIdx * kStepSize)) {
            qCritical() << "[filesystem_helper::moveFsForward] Failed to seek to target position:" << (lastIdx * kStepSize) << "for device:" << dev;
            return false;
        }

        if (devFile.write(buf, readed) != readed) {
            qCritical() << "[filesystem_helper::moveFsForward] Read/write size mismatch during operation for device:" << dev;
            return false;
        }

        if (!devFile.flush() || fsync(devFile.handle()) != 0) {
            qCritical() << "[filesystem_helper::moveFsForward] Failed to flush device file for device:" << dev;
            return false;
        }

        // record current index.
        QString pos = "," + QString::number(curIdx);
        logFile.write(pos.toLocal8Bit());
        if (!logFile.flush() || fsync(logFile.handle()) != 0) {
            qCritical() << "[filesystem_helper::moveFsForward] Failed to flush log file for device:" << dev;
            return false;
        }
    }

    ::remove(logFile.fileName().toStdString().c_str());
    qInfo() << "[filesystem_helper::moveFsForward] Filesystem forward move completed successfully, log file removed for device:" << dev;
    // ::system("udevadm trigger");
    return true;
}

void filesystem_helper::remountBoot()
{
    qInfo() << "[filesystem_helper::remountBoot] Starting boot partition remount operation";
    
    // find boot partition first.
    QString bootDev, bootPath, fsType;
    struct fstab *fs;
    setfsent();
    while ((fs = getfsent()) != nullptr) {
        if (QString(fs->fs_file) == "/boot") {
            bootDev = fs->fs_spec;
            bootPath = fs->fs_file;
            fsType = fs->fs_vfstype;
            break;
        }
    }
    endfsent();

    if (bootDev.isEmpty()) {
        qWarning() << "[filesystem_helper::remountBoot] Boot device not found in fstab";
        return;
    }
    qInfo() << "[filesystem_helper::remountBoot] Boot device found:" << bootDev << "path:" << bootPath << "filesystem:" << fsType;

    int r = mount(bootDev.toStdString().c_str(),
                  bootPath.toStdString().c_str(),
                  fsType.toStdString().c_str(),
                  MS_REMOUNT,
                  nullptr);
    if (r == 0) {
        qInfo() << "[filesystem_helper::remountBoot] Boot partition remounted successfully";
    } else {
        qWarning() << "[filesystem_helper::remountBoot] Boot partition remount failed, error code:" << r;
    }

    const QString path = "/boot/usec-crypt";
    auto ok = QDir().mkpath(path);
    bool dirExists = QFile(path).exists();
    qInfo() << "[filesystem_helper::remountBoot] USec crypt directory creation - path:" << path << "created:" << ok << "exists:" << dirExists;
}
