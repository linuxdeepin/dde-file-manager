/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DFMBLOCKDEVICE_H
#define DFMBLOCKDEVICE_H

#include <dfmglobal.h>

#include <QObject>
#include <QVariantMap>
#include <QDBusUnixFileDescriptor>

class QDBusObjectPath;

DFM_BEGIN_NAMESPACE

class DFMBlockDevicePrivate;
class DFMBlockDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMBlockDevice)

    Q_PROPERTY(QString path READ path CONSTANT FINAL)
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)
    Q_PROPERTY(bool hasFileSystem READ hasFileSystem NOTIFY hasFileSystemChanged)
    Q_PROPERTY(bool hasPartition READ hasPartition NOTIFY hasPartitionChanged)
    Q_PROPERTY(bool isEncrypted READ isEncrypted NOTIFY isEncryptedChanged)
    Q_PROPERTY(bool isLoopDevice READ isLoopDevice)
    Q_PROPERTY(QList<QPair<QString, QVariantMap>> configuration READ configuration NOTIFY configurationChanged)
    Q_PROPERTY(QString cryptoBackingDevice READ cryptoBackingDevice NOTIFY cryptoBackingDeviceChanged)
    Q_PROPERTY(QByteArray device READ device)
    Q_PROPERTY(qulonglong deviceNumber READ deviceNumber)
    Q_PROPERTY(QString drive READ drive)
    Q_PROPERTY(bool hintAuto READ hintAuto NOTIFY hintAutoChanged)
    Q_PROPERTY(QString hintIconName READ hintIconName NOTIFY hintIconNameChanged)
    Q_PROPERTY(bool hintIgnore READ hintIgnore NOTIFY hintIgnoreChanged)
    Q_PROPERTY(QString hintName READ hintName NOTIFY hintNameChanged)
    Q_PROPERTY(bool hintPartitionable READ hintPartitionable NOTIFY hintPartitionableChanged)
    Q_PROPERTY(QString hintSymbolicIconName READ hintSymbolicIconName NOTIFY hintSymbolicIconNameChanged)
    Q_PROPERTY(bool hintSystem READ hintSystem)
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString idLabel READ idLabel NOTIFY idLabelChanged)
    Q_PROPERTY(QString idType READ idType NOTIFY idTypeChanged)
    Q_PROPERTY(FSType fsType READ fsType NOTIFY fsTypeChanged)
    Q_PROPERTY(QString idUUID READ idUUID NOTIFY idUUIDChanged)
    Q_PROPERTY(QString idUsage READ idUsage NOTIFY idUsageChanged)
    Q_PROPERTY(QString idVersion READ idVersion NOTIFY idVersionChanged)
    Q_PROPERTY(QString mDRaid READ mDRaid NOTIFY mDRaidChanged)
    Q_PROPERTY(QString mDRaidMember READ mDRaidMember NOTIFY mDRaidMemberChanged)
    Q_PROPERTY(QByteArray preferredDevice READ preferredDevice NOTIFY preferredDeviceChanged)
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(qulonglong size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QByteArrayList symlinks READ symlinks NOTIFY symlinksChanged)
    Q_PROPERTY(QStringList userspaceMountOptions READ userspaceMountOptions NOTIFY userspaceMountOptionsChanged)
    // of FileSystem
    Q_PROPERTY(QByteArrayList mountPoints READ mountPoints NOTIFY mountPointsChanged)
    // of ParitionTable
    Q_PROPERTY(PTType ptType READ ptType NOTIFY ptTypeChanged)
    // of Encrypted
    Q_PROPERTY(QList<QPair<QString, QVariantMap>> childConfiguration READ childConfiguration NOTIFY childConfigurationChanged)

public:
    // partition table type
    enum PTType {
        InvalidPT,
        MBR,
        GPT,
        UnknowPT
    };

    Q_ENUM(PTType)

    enum FSType {
        InvalidFS,
        UnknowFS,
        ext2,
        ext3,
        ext4,
        fat12,
        fat16,
        fat32,
        btrfs,
        f2fs,
        hfs_plus,
        minix,
        nilfs2,
        ntfs,
        reiser4,
        vfat,
        iso9660,
        jfs,
        xfs,
        swap,
        LVM2_member,
        crypto_LUKS
    };

    Q_ENUM(FSType)

    ~DFMBlockDevice();

    bool watchChanges() const;

    QString path() const;
    QList<QPair<QString, QVariantMap>> configuration() const;
    QString cryptoBackingDevice() const;
    QByteArray device() const;
    qulonglong deviceNumber() const;
    QString drive() const;
    bool hintAuto() const;
    QString hintIconName() const;
    bool hintIgnore() const;
    QString hintName() const;
    bool hintPartitionable() const;
    QString hintSymbolicIconName() const;
    bool hintSystem() const;
    QString id() const;
    QString idLabel() const;
    QString idType() const;
    FSType fsType() const;
    QString idUUID() const;
    QString idUsage() const;
    QString idVersion() const;
    QString mDRaid() const;
    QString mDRaidMember() const;
    QByteArray preferredDevice() const;
    bool readOnly() const;
    qulonglong size() const;
    QByteArrayList symlinks() const;
    QStringList userspaceMountOptions() const;

    bool hasFileSystem() const;
    bool hasPartition() const;
    bool isEncrypted() const;
    bool isLoopDevice() const;

    static bool hasFileSystem(const QString &path);
    static bool hasPartition(const QString &path);
    static bool isEncrypted(const QString &path);

    QByteArrayList mountPoints() const;
    PTType ptType() const;

    QList<QPair<QString, QVariantMap>> childConfiguration() const;

public Q_SLOTS:
    void setWatchChanges(bool watchChanges);

    void addConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options);
    void format(const QString &type, const QVariantMap &options);
    void format(const FSType &type, const QVariantMap &options);
    QList<QPair<QString, QVariantMap>> getSecretConfiguration(const QVariantMap &options);
    QDBusUnixFileDescriptor openDevice(const QString &mode, const QVariantMap &options);
    QDBusUnixFileDescriptor openForBackup(const QVariantMap &options);
    QDBusUnixFileDescriptor openForBenchmark(const QVariantMap &options);
    QDBusUnixFileDescriptor openForRestore(const QVariantMap &options);
    void removeConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options);
    void rescan(const QVariantMap &options);
    void updateConfigurationItem(const QPair<QString, QVariantMap> &old_item, const QPair<QString, QVariantMap> &new_item, const QVariantMap &options);

    // of Filesystem
    QString mount(const QVariantMap &options);
    void unmount(const QVariantMap &options);
    bool canSetLabel() const;
    void setLabel(const QString &label, const QVariantMap &options);

    // of Encrypted
    void changePassphrase(const QString &passphrase, const QString &new_passphrase, const QVariantMap &options);
    void lock(const QVariantMap &options);
    void unlock(const QString &passphrase, const QVariantMap &options);

Q_SIGNALS:
    void hasFileSystemChanged(bool hasFileSystem);
    void hasPartitionChanged(bool hasPartition);
    void isEncryptedChanged(bool isEncrypted);
    void configurationChanged(QList<QPair<QString, QVariantMap>> configuration);
    void cryptoBackingDeviceChanged(QString cryptoBackingDevice);
    void hintAutoChanged(bool hintAuto);
    void hintIconNameChanged(QString hintIconName);
    void hintIgnoreChanged(bool hintIgnore);
    void hintNameChanged(QString hintName);
    void hintPartitionableChanged(bool hintPartitionable);
    void hintSymbolicIconNameChanged(QString hintSymbolicIconName);
    void idLabelChanged(QString idLabel);
    void idTypeChanged(QString idType);
    void fsTypeChanged();
    void idUUIDChanged(QString idUUID);
    void idUsageChanged(QString idUsage);
    void idVersionChanged(QString idVersion);
    void mDRaidChanged(QString mDRaid);
    void mDRaidMemberChanged(QString mDRaidMember);
    void preferredDeviceChanged(QByteArray preferredDevice);
    void readOnlyChanged(bool readOnly);
    void sizeChanged(qulonglong size);
    void symlinksChanged(QByteArrayList symlinks);
    void userspaceMountOptionsChanged(QStringList userspaceMountOptions);
    void ptTypeChanged();
    void mountPointsChanged(const QByteArrayList &mountPoints);
    void childConfigurationChanged(QList<QPair<QString, QVariantMap>> childConfiguration);

protected:
    explicit DFMBlockDevice(const QString &path, QObject *parent = nullptr);
    explicit DFMBlockDevice(DFMBlockDevicePrivate &dd, const QString &path, QObject *parent = nullptr);

    QScopedPointer<DFMBlockDevicePrivate> d_ptr;

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties);
    void onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties);
//    Q_PRIVATE_SLOT(d_ptr, void _q_onPropertiesChanged(const QString &, const QVariantMap &))

    friend class DFMDiskManager;
};

DFM_END_NAMESPACE

#endif // DFMBLOCKDEVICE_H
