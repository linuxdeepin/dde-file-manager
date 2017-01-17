#include "partitionmanager.h"
#include "partition.h"
#include "command.h"
#include <QMetaObject>
#include <QMetaEnum>
#include <QString>
#include <QDebug>


namespace PartMan {

int PartitionManager::getMaxNameLengthByTypeString(const QString &typeStr)
{
    QMetaEnum fsType = QMetaEnum::fromType<FsType>();
    FsType type = (FsType)fsType.keyToValue(typeStr.toStdString().data());
    return getMaxNameLengthByType(type);
}

int PartitionManager::getMaxNameLengthByType(const PartitionManager::FsType &type)
{
    switch (type) {
    case Btrfs:
        return 255;
        break;
    case Efi:
        return 255;
        break;
    case Ext2:
        return 16;
        break;
    case Ext3:
        return 16;
        break;
    case Ext4:
        return 16;
        break;
    case F2fs:
        return 19;
        break;
    case Fat16:
        return 11;
        break;
    case Fat32:
        return 11;
        break;
    case Hfs:
        return 27;
        break;
    case Hfsplus:
        return 63;
        break;
    case Jfs:
        return 11;
        break;
    case Linuxswap:
        return 15;
        break;
    case Lvm2pv:
        return 255;
        break;
    case Nilfs2:
        return 1;
        break;
    case Ntfs:
        return 128;
        break;
    case Reiser4:
        return 16;
        break;
    case Reiserfs:
        return 16;
        break;
    case Xfs:
        return 16;
        break;
    default:
        return 0;
        break;
    }
}

PartitionManager::PartitionManager(QObject *parent) : QObject(parent)
{

}

bool PartitionManager::mkfs(const Partition &partition)
{
    return mkfs(partition.path(), partition.fs(), partition.label());
}

bool PartitionManager::mkfs(const QString &path, const QString &fs, const QString &label)
{
    QString key = fs.left(1).toUpper() + fs.right(fs.length()-1);
    QString methodKey = QString("actionFormat%1").arg(key);
    QString methodSignature = QString("actionFormat%1(QString,QString)").arg(key);

    const QMetaObject* metaObject = this->metaObject();
//    QStringList methods;
//    for(int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i){
//        methods << QString::fromLatin1(metaObject->method(i).methodSignature());
//    }
//    qDebug() << methods;

    bool ret = false;
    if (metaObject->indexOfSlot(methodSignature.toLocal8Bit().constData()) != -1){
        QMetaObject::invokeMethod(this,
                                  methodKey.toLocal8Bit().constData(),
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(QString, path),
                                  Q_ARG(QString, label));
    }else{
        qWarning() << "PartitionManager has no method:" << methodSignature;
    }
    return ret;
}

bool PartitionManager::actionFormatBtrfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();

    return actionFormatBtrfs(path, label);
}

bool PartitionManager::actionFormatBtrfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.btrfs", {"-f", path}, output, err);
    } else {
      // Truncate label size.
      const QString real_label = label.left(255);
      ok = SpawnCmd("mkfs.btrfs", {"-f", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatBtrfs() error:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatEfi(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();

    return actionFormatEfi(path, label);
}

bool PartitionManager::actionFormatEfi(const QString &path, const QString &label)
{
    return actionFormatFat32(path, label);
}

bool PartitionManager::actionFormatExt2(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatExt2(path, label);
}

bool PartitionManager::actionFormatExt2(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.ext2", {"-F", path}, output, err);
    } else {
      const QString real_label = label.left(16);
      ok = SpawnCmd("mkfs.ext2", {"-F", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatExt2() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatExt3(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatExt3(path, label);
}

bool PartitionManager::actionFormatExt3(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.ext3", {"-F", path}, output, err);
    } else {
      const QString real_label = label.left(16);
      ok = SpawnCmd("mkfs.ext3", {"-F", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatExt3() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatExt4(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatExt4(path, label);
}

bool PartitionManager::actionFormatExt4(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.ext4", {"-F", path}, output, err);
    } else {
      const QString real_label = label.left(16);
      ok = SpawnCmd("mkfs.ext4", {"-F", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatExt4() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatF2fs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatF2fs(path, label);
}

bool PartitionManager::actionFormatF2fs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.f2fs", {path}, output, err);
    } else {
      const QString real_label = label.left(19);
      ok = SpawnCmd("mkfs.f2fs", {QString("-l%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatF2fs() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatFat16(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatFat16(path, label);
}

bool PartitionManager::actionFormatFat16(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.msdos", {"-F16", "-v", "-I", path}, output, err);
    } else {
      const QString real_label = label.left(11);
      ok = SpawnCmd("mkfs.msdos",
                    {"-F16", "-v", "-I", QString("-n%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatFat16() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatFat32(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatFat32(path, label);
}

bool PartitionManager::actionFormatFat32(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.msdos", {"-F32", "-v", "-I", path}, output, err);
    } else {
      const QString real_label = label.left(11);
      ok = SpawnCmd("mkfs.msdos",
                    {"-F32", "-v", "-I", QString("-n%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatFat32() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatHfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatHfs(path, label);
}

bool PartitionManager::actionFormatHfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("hformat", {path}, output, err);
    } else {
      const QString real_label = label.left(27);
      ok = SpawnCmd("hformat", {QString("-l%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatHfs() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatHfsplus(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatHfsplus(path, label);
}

bool PartitionManager::actionFormatHfsplus(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.hfsplus", {path}, output, err);
    } else {
      const QString real_label = label.left(63);
      ok = SpawnCmd("mkfs.hfsplus", {QString("-v%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatHfsPlus() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatJfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatJfs(path, label);
}

bool PartitionManager::actionFormatJfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.jfs", {"-q", path}, output, err);
    } else {
      const QString real_label = label.left(11);
      ok = SpawnCmd("mkfs.jfs", {"-q", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatJfs() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatLinuxswap(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatLinuxswap(path, label);
}

bool PartitionManager::actionFormatLinuxswap(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkswap", {path}, output, err);
    } else {
      const QString real_label = label.left(15);
      ok = SpawnCmd("mkswap", {QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatLinuxSwap() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatLvm2pv(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatLvm2pv(path, label);
}

bool PartitionManager::actionFormatLvm2pv(const QString &path, const QString &label)
{
    return actionFormatUnknown(path, label);
}

bool PartitionManager::actionFormatNilfs2(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatNilfs2(path, label);
}

bool PartitionManager::actionFormatNilfs2(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.nilfs2", {path}, output, err);
    } else {
      const QString real_label = label.left(1);
      ok = SpawnCmd("mkfs.nilfs2", {QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatNilfs2() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatNtfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatNtfs(path, label);
}

bool PartitionManager::actionFormatNtfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkntfs", {"-Q", "-v", "-F", path}, output, err);
    } else {
      const QString real_label = label.left(128);
      ok = SpawnCmd("mkntfs",
                    {"-Q", "-v", "-F", "-L", QString("%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatNTFS() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatReiser4(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatReiser4(path, label);
}

bool PartitionManager::actionFormatReiser4(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.reiser4", {"--force", "--yes", path}, output, err);
    } else {
      const QString real_label = label.left(16);
      ok = SpawnCmd("mkfs.reiser4",
                   {"--force", "--yes",
                    QString("--label%1").arg(real_label), path},
                   output, err);
    }
    if (!ok) {
      qCritical() << "FormatReiser4() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatReiserfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatReiserfs(path, label);
}

bool PartitionManager::actionFormatReiserfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkreiserfs", {"-f", "-f", path}, output, err);
    } else {
      const QString real_label = label.left(16);
      ok = SpawnCmd("mkreiserfs",
                    {"-f", "-f", QString("--label%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatReiserfs() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatXfs(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatXfs(path, label);
}

bool PartitionManager::actionFormatXfs(const QString &path, const QString &label)
{
    QString output;
    QString err;
    bool ok;
    if (label.isEmpty()) {
      ok = SpawnCmd("mkfs.xfs", {"-f", path}, output, err);
    } else {
      const QString real_label = label.left(12);
      ok = SpawnCmd("mkfs.xfs", {"-f", QString("-L%1").arg(real_label), path},
                    output, err);
    }
    if (!ok) {
      qCritical() << "FormatXfs() err:" << err;
    }
    return ok;
}

bool PartitionManager::actionFormatUnknown(const Partition &partition)
{
    QString path = partition.path();
    QString label = partition.label();
    return actionFormatUnknown(path, label);
}

bool PartitionManager::actionFormatUnknown(const QString &path, const QString &label)
{
    qDebug() << path << label << "unsupport format";
    return false;
}

}
