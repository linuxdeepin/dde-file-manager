#include "readusagemanager.h"
#include "string_util.h"
#include "structs.h"
#include "command.h"
#include "partition.h"
#include <QMetaObject>
#include <QMetaEnum>
#include <QString>
#include <QDebug>

namespace PartMan {

ReadUsageManager::ReadUsageManager(QObject *parent) : QObject(parent)
{

}

qlonglong ReadUsageManager::ParseBtrfsUnit(const QString &value)
{
    const float pref = RegexpLabel("^(\\d+\\.?\\d+)", value).toFloat();
    if (value.contains("KiB")) {
        return static_cast<qlonglong>(pref * kKibiByte);
    }
    if (value.contains("MiB")) {
        return static_cast<qlonglong>(pref * kMebiByte);
    }
    if (value.contains("GiB")) {
        return static_cast<qlonglong>(pref * kGibiByte);
    }
    if (value.contains("TiB")) {
        return static_cast<qlonglong>(pref * kTebiByte);
    }
    if (value.contains("PiB")) {
        return static_cast<qlonglong>(pref * kPebiByte);
    }
    return -1;
}

bool ReadUsageManager::readUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "Start read usage of " << path;
    freespace = 0;
    total = 0;
    Partition p = Partition::getPartitionByDevicePath(path);
    return readUsage(path, p.fs(), freespace, total);
}

bool ReadUsageManager::readUsage(const QString &path, const QString &fs, qlonglong &freespace, qlonglong &total)
{
    if (fs.isEmpty()){
        return false;
    }
    QString _fs = fs;
    if (_fs == "vfat")
         _fs = "fat16";
    QString key = _fs.left(1).toUpper() + _fs.right(_fs.length()-1);
    QString methodKey = QString("read%1Usage").arg(key);
    QString methodSignature = QString("read%1Usage(QString,qlonglong&,qlonglong&)").arg(key);

    const QMetaObject* metaObject = this->metaObject();
//    QStringList methods;
//    for(int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i){
//        methods << QString::fromLatin1(metaObject->method(i).methodSignature());
//    }
//    qDebug() << methods << methods.contains(methodSignature);
//    qDebug() << methodSignature;
    bool ret = false;
    if (metaObject->indexOfSlot(methodSignature.toLocal8Bit().constData()) != -1){
        QMetaObject::invokeMethod(this,
                                  methodKey.toLocal8Bit().constData(),
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(QString, path),
                                  Q_ARG(qlonglong&, freespace),
                                  Q_ARG(qlonglong&, total));
    }else{
        qWarning() << "PartitionManager has no method:" << methodSignature;
    }
    return ret;
}

bool ReadUsageManager::readBtrfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("btrfs", {"filesystem", "show", path}, output)) {
        return false;
    }
    QString total_str, used_str;
    for (const QString& line : output.split('\n')) {
        if (line.contains(path)) {
            total_str = RegexpLabel("size\\s*([^\\s]*)\\s", line);
        } else if (line.contains("Total devices")) {
            used_str = RegexpLabel("used\\s*([^\\s]*)", line);
        }
    }
    total = ParseBtrfsUnit(total_str);
    freespace = total - ParseBtrfsUnit(used_str);
    return (total > -1 && freespace > -1);
}

bool ReadUsageManager::readEfiUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    return readFat16Usage(path, freespace, total);
}

bool ReadUsageManager::readExt2Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("dumpe2fs", {"-h", path}, output)) {
        return false;
    }

    int block_size = 0;
    qlonglong total_blocks = 0;
    qlonglong free_blocks = 0;
    for (const QString& line : output.split('\n')) {
        if (line.contains("Block count:")) {
            const QString m = RegexpLabel("Block count:\\s+(\\d+)", line);
            if (!m.isEmpty()) {
                total_blocks = m.toLongLong();
            }
        } else if (line.contains("Free blocks:")) {
            const QString m = RegexpLabel("Free blocks:\\s+(\\d+)", line);
            if (!m.isEmpty()) {
                free_blocks = m.toLongLong();
            }
        } else if (line.contains("Block size:")) {
            const QString m = RegexpLabel("Block size:\\s+(\\d+)", line);
            if (!m.isEmpty()) {
                block_size = m.toInt();
            }
        }
    }
    freespace = block_size * free_blocks;
    total = block_size * total_blocks;
    return true;
}

bool ReadUsageManager::readExt3Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    return readExt2Usage(path, freespace, total);
}

bool ReadUsageManager::readExt4Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    return readExt2Usage(path, freespace, total);
}

bool ReadUsageManager::readF2fsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "unsupport F2fs fs type usage read";
    return readUnknownUsage(path, freespace, total);
}

bool ReadUsageManager::readFat16Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output, err;
    SpawnCmd("dosfsck", {"-n", "-v", path}, output, err);
      // NOTE(xushaohua): `dosfsck` returns 1 on success, so we check its error
      // message instead.
    if (!err.isEmpty()) {
        qWarning() << "dosfsck failed:" << err;
        return false;
    }

    int cluster_size = 0;
    qlonglong start_byte = 0;
    qlonglong total_clusters = 0;
    qlonglong used_clusters = 0;

    for (const QString& line : output.split('\n')) {
        if (line.contains("bytes per cluster")) {
            cluster_size = line.trimmed().split(' ').at(0).trimmed().toInt();
        } else if (line.contains("Data area starts at")) {
            start_byte = line.split(' ').at(5).toLongLong();
        } else if (line.contains(path)) {
            const QStringList parts = line.split(' ').at(3).split('/');
            total_clusters = parts.at(1).toLongLong();
            used_clusters = parts.at(0).toLongLong();
        }
    }

    total = total_clusters * cluster_size;
    freespace = total - start_byte - used_clusters * cluster_size;

    return true;
}

bool ReadUsageManager::readFat32Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    return readFat16Usage(path, freespace, total);
}

bool ReadUsageManager::readHfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "unsupport  Hfs fs type usage read";
    return readUnknownUsage(path, freespace, total);
}

bool ReadUsageManager::readHfsplusUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "unsupport Hfsplus fs type usage read";
    return readUnknownUsage(path, freespace, total);
}

bool ReadUsageManager::readJfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    const QString param(QString("echo dm | jfs_debugfs %1").arg(path));
    if (!SpawnCmd("sh", {"-c", param}, output)) {
        return false;
    }

    int block_size = 0;
    qlonglong total_blocks = 0;
    qlonglong free_blocks = 0;
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("Aggregate Block Size:")) {
            block_size = line.split(':').at(1).trimmed().toInt();
        } else if (line.contains("dn_mapsize:")) {
            const QString item = RegexpLabel("dn_mapsize:\\s*([^\\s]+)", line);
            total_blocks = item.toLongLong(nullptr, 16);
        } else if (line.contains("dn_nfree:")) {
            const QString item = RegexpLabel("dn_nfree:\\s*([^\\s]+)", line);
            free_blocks = item.toLongLong(nullptr, 16);
        }
    }

    if (free_blocks > 0 && total_blocks > 0 && block_size > 0) {
        freespace = free_blocks * block_size;
        total = total_blocks * block_size;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readLinuxswapUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "unsupport linxu swap fs type usage read";
    return readUnknownUsage(path, freespace, total);
}

bool ReadUsageManager::readLvm2pvUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    qDebug() << "unsupport Lvm2pv fs type usage read";
    return readUnknownUsage(path, freespace, total);
}

bool ReadUsageManager::readNilfs2Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("nilfs-tune", {"-l", path}, output)) {
        return false;
    }

    int block_size = 0;
    qlonglong free_blocks = 0;
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("Block size:")) {
          block_size = line.split(':').last().trimmed().toInt();
        } else if (line.startsWith("Device size:")) {
          total = line.split(':').last().trimmed().toLongLong();
        } else if (line.startsWith("Free blocks count:")) {
          free_blocks = line.split(':').last().trimmed().toLongLong();
        }
    }

    if (free_blocks > 0 && block_size > 0) {
        freespace = free_blocks * block_size;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readNtfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("ntfsinfo", {"-mf", path}, output)) {
        return false;
    }
    int cluster_size = 0;
    qlonglong free_clusters = 0;
    qlonglong total_clusters = 0;
    for (const QString& line : output.split('\n')) {
        if (line.contains("Cluster Size:")) {
            const QString m = RegexpLabel("Cluster Size:\\s+(\\d+)", line);
            if (!m.isEmpty()) {
                cluster_size = m.toInt();
            }
        } else if (line.contains("Volume Size in Clusters:")) {
            const QString m = RegexpLabel("Volume Size in Clusters:\\s+(\\d+)",
                                              line);
            if (!m.isEmpty()) {
                total_clusters = m.toLongLong();
            }
        } else if (line.contains("Free Clusters:")) {
            const QString m = RegexpLabel("Free Clusters:\\s+(\\d+)", line);
            if (!m.isEmpty()) {
                free_clusters = m.toLongLong();
            }
        }
    }

    if (free_clusters > 0 && total_clusters > 0 && cluster_size > 0) {
        freespace = cluster_size * free_clusters;
        total = cluster_size * total_clusters;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readReiser4Usage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("debugfs.reiser4", {"--force", "--yes", path}, output)) {
        return false;
    }

    int block_size = 0;
    qlonglong free_blocks = 0;
    qlonglong total_blocks = 0;
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("blksize:")) {
            block_size = line.split(':').at(1).trimmed().toInt();
        } else if (line.startsWith("blocks:")) {
            total_blocks = line.split(':').at(1).trimmed().toLongLong();
        } else if (line.startsWith("free blocks:")) {
            free_blocks = line.split(':').at(1).trimmed().toLongLong();
        }
    }

    if (free_blocks > 0 && total_blocks > 0 && block_size > 0) {
        total = total_blocks * block_size;
        freespace = free_blocks * block_size;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readReiserfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("debugreiserfs", {"-d", path}, output)) {
        return false;
    }

    qlonglong total_blocks = 0;
    qlonglong free_blocks = 0;
    int block_size = 0;
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("Count of blocks on the device:")) {
            total_blocks = line.split(':').last().trimmed().toLongLong();
        } else if (line.startsWith("Blocksize:")) {
            block_size = line.split(':').last().trimmed().toInt();
        } else if (line.startsWith("Free blocks (count of blocks")) {
            free_blocks = line.split(':').last().trimmed().toLongLong();
        }
    }

    if (free_blocks > 0 && total_blocks > 0 && block_size > 0) {
        freespace = free_blocks * block_size;
        total = total_blocks * block_size;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readXfsUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    QString output;
    if (!SpawnCmd("xfs_db", {"-c sb", "-c print", "-r", path}, output)) {
        return false;
    }
    if (output.isEmpty()) {
        return false;
    }

    int block_size = 0;
    qlonglong total_blocks = 0;
    qlonglong free_blocks = 0;
    for (const QString& line : output.split('\n')) {
        if (line.contains("fdblocks")) {
            free_blocks = line.split('=').last().trimmed().toLongLong();
        } else if (line.contains("dblocks")) {
            total_blocks = line.split('=').last().trimmed().toLongLong();
        } else if (line.contains("blocksize")) {
            block_size = line.split('=').last().trimmed().toInt();
        }
    }

    if (free_blocks > 0 && total_blocks > 0 && block_size > 0) {
        freespace = free_blocks * block_size;
        total = total_blocks * block_size;
        return true;
    } else {
        return false;
    }
}

bool ReadUsageManager::readUnknownUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    freespace = 0;
    total = 0;
    qDebug() << "unsupport fs type" << path << freespace << total;
    return false;
}
}
