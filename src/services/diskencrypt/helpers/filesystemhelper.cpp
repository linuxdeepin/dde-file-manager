// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "e2fsck failed!"
                   << ret;
        return false;
    }

    auto size = blockdev_helper::devDeviceSize(device) / 1024 / 1024;
    qInfo() << device << "will be resize to" << (size - 32) << "MB";
    cmd = QString("resize2fs %1 %2M").arg(device).arg(size - 32);
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "resize2fs failed"
                   << ret;
        return false;
    }

    return true;
}

bool filesystem_helper::expandFileSystem_ext(const QString &device)
{
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "e2fsck failed!"
                   << ret;
        return false;
    }

    cmd = QString("resize2fs %1").arg(device);
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "resize2fs failed"
                   << ret;
        return false;
    }

    return true;
}

bool filesystem_helper::moveFsForward(const QString &dev)
{
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
        if (!logFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            qWarning() << "cannot create fsmove log file!" << dev;
            return false;
        }
        logFile.close();
    }

    if (!logFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)
        || !devFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        qWarning() << "log file or device file cannot be opened." << dev;
        return false;
    }

    auto size = blockdev_helper::devDeviceSize(dev);
    if (size == 0) {
        qWarning() << "cannot obtain device size!" << dev;
        return false;
    }

    auto count = size / kStepSize;
    if (size % kStepSize == 0) count -= 1;

    auto log = logFile.readAll().split(',');
    auto lastIdx = (log.count() > 0) ? log.last().toULongLong() : 0;   // breakpoint

    for (auto curIdx = lastIdx + 1;
         curIdx <= count;
         ++curIdx, ++lastIdx) {
        if (curIdx * kStepSize >= size) {
            qWarning() << "The seek pos is greater than part size!";
            break;
        }

        // seek current move position
        if (!devFile.seek(curIdx * kStepSize)) {
            qWarning() << "seek pos failed!" << curIdx * kStepSize;
            return false;
        }

        // read next step
        memset(buf, 0, kStepSize);
        quint64 readed = devFile.read(buf, kStepSize);

        // write to previous step position.
        if (!devFile.seek(lastIdx * kStepSize)) {
            qWarning() << "seek target failed!" << lastIdx * kStepSize;
            return false;
        }

        if (devFile.write(buf, readed) != readed) {
            qWarning() << "read write size not match!";
            return false;
        }

        if (!devFile.flush() || fsync(devFile.handle()) != 0) {
            qWarning() << "cannot flush device file!" << dev;
            return false;
        }

        // recore current index.
        QString pos = "," + QString::number(curIdx);
        logFile.write(pos.toLocal8Bit());
        if (!logFile.flush() || fsync(logFile.handle()) != 0) {
            qWarning() << "cannot flush log file!" << dev;
            return false;
        }
    }

    ::remove(logFile.fileName().toStdString().c_str());
    // ::system("udevadm trigger");
    return true;
}

void filesystem_helper::remountBoot()
{
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
        qWarning() << "cannot find BOOT device!";
        return;
    }
    qInfo() << "boot device found:" << bootDev;

    int r = mount(bootDev.toStdString().c_str(),
                  bootPath.toStdString().c_str(),
                  fsType.toStdString().c_str(),
                  MS_REMOUNT,
                  nullptr);
    qInfo() << "/boot remounted:" << r;

    const QString path = "/boot/usec-crypt";
    auto ok = QDir().mkpath(path);
    qInfo() << path << "created:" << ok
            << "dir exists:" << QFile(path).exists();
}
