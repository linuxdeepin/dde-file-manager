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
#include "dfmblockdevice.h"
#include "private/dfmblockdevice_p.h"
#include "udisks2_interface.h"
#include "objectmanager_interface.h"

DFM_BEGIN_NAMESPACE

DFMBlockDevicePrivate::DFMBlockDevicePrivate(DFMBlockDevice *qq)
    : q_ptr(qq)
{

}

void DFMBlockDevice::onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties)
{
    Q_D(DFMBlockDevice);

    const QString &path = object_path.path();

    if (path != d->dbus->path())
        return;

    if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Filesystem"))) {
        Q_EMIT hasFileSystemChanged(true);
    }

    if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Partition"))) {
        Q_EMIT hasPartitionChanged(true);
    }

    if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Encrypted"))) {
        Q_EMIT isEncryptedChanged(true);
    }
}

void DFMBlockDevice::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    Q_D(DFMBlockDevice);

    const QString &path = object_path.path();

    if (path != d->dbus->path())
        return;

    for (const QString &i : interfaces) {
        if (i == QStringLiteral(UDISKS2_SERVICE ".Filesystem")) {
            Q_EMIT hasFileSystemChanged(false);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Partition")) {
            Q_EMIT hasPartitionChanged(false);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Encrypted")) {
            Q_EMIT isEncryptedChanged(false);
        }
    }
}

void DFMBlockDevice::onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties)
{
    if (interface.endsWith(".PartitionTable")) {
        auto begin = changed_properties.begin();

        while (begin != changed_properties.constEnd()) {
            if (begin.key() == "Type") {
                Q_EMIT ptTypeChanged();
                break;
            }

            ++begin;
        }
    } else {
        auto begin = changed_properties.begin();

        for (; begin != changed_properties.constEnd(); ++begin) {
            QString property_name = begin.key();

            int pindex = this->metaObject()->indexOfProperty(property_name.toLatin1().constData());

            if (pindex < 0) {
                property_name[0] = property_name.at(0).toLower();

                pindex = this->metaObject()->indexOfProperty(property_name.toLatin1().constData());
            }

            if (pindex < 0)
                continue;

            const QMetaProperty &mp = this->metaObject()->property(pindex);

            if (!mp.hasNotifySignal())
                continue;

            mp.notifySignal().invoke(this, QGenericArgument(begin.value().typeName(), begin.value().constData()));
        }
    }
}

DFMBlockDevice::~DFMBlockDevice()
{

}

bool DFMBlockDevice::watchChanges() const
{
    Q_D(const DFMBlockDevice);

    return d->watchChanges;
}

/*!
 * \brief Get the block device (patition) DBus path.
 *
 * \return the DBus path
 */
QString DFMBlockDevice::path() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->path();
}

QList<QPair<QString, QVariantMap> > DFMBlockDevice::configuration() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->configuration();
}

QString DFMBlockDevice::cryptoBackingDevice() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->cryptoBackingDevice().path();
}

/*!
 * \brief Get the device path, like `/dev/sda1`
 *
 * \return Device path in QByteArray.
 */
QByteArray DFMBlockDevice::device() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->device();
}

qulonglong DFMBlockDevice::deviceNumber() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->deviceNumber();
}

/*!
 * \brief Get the disk device dbus path
 *
 * \return a dbus path in QString
 */
QString DFMBlockDevice::drive() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->drive().path();
}

bool DFMBlockDevice::hintAuto() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintAuto();
}

QString DFMBlockDevice::hintIconName() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintIconName();
}

bool DFMBlockDevice::hintIgnore() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintIgnore();
}

QString DFMBlockDevice::hintName() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintName();
}

bool DFMBlockDevice::hintPartitionable() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintPartitionable();
}

QString DFMBlockDevice::hintSymbolicIconName() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintSymbolicIconName();
}

bool DFMBlockDevice::hintSystem() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->hintSystem();
}

QString DFMBlockDevice::id() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->id();
}

QString DFMBlockDevice::idLabel() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->idLabel();
}

QString DFMBlockDevice::idType() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->idType();
}

DFMBlockDevice::FSType DFMBlockDevice::fsType() const
{
    const QString &fs_type = idType();

    if (fs_type.isEmpty())
        return InvalidFS;

    if (fs_type == "hfs+")
        return hfs_plus;

    bool ok = false;
    const QMetaEnum me = QMetaEnum::fromType<FSType>();

    int value = me.keyToValue(fs_type.toLatin1().constData(), &ok);

    if (!ok) {
        return UnknowFS;
    }

    return static_cast<FSType>(value);
}

QString DFMBlockDevice::idUUID() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->idUUID();
}

QString DFMBlockDevice::idUsage() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->idUsage();
}

QString DFMBlockDevice::idVersion() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->idVersion();
}

QString DFMBlockDevice::mDRaid() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->mDRaid().path();
}

QString DFMBlockDevice::mDRaidMember() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->mDRaidMember().path();
}

QByteArray DFMBlockDevice::preferredDevice() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->preferredDevice();
}

bool DFMBlockDevice::readOnly() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->readOnly();
}

qulonglong DFMBlockDevice::size() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->size();
}

QByteArrayList DFMBlockDevice::symlinks() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->symlinks();
}

QStringList DFMBlockDevice::userspaceMountOptions() const
{
    Q_D(const DFMBlockDevice);

    return d->dbus->userspaceMountOptions();
}

bool DFMBlockDevice::hasFileSystem() const
{
    Q_D(const DFMBlockDevice);

    return hasFileSystem(d->dbus->path());
}

bool DFMBlockDevice::hasPartition() const
{
    Q_D(const DFMBlockDevice);

    return hasPartition(d->dbus->path());
}

bool DFMBlockDevice::isEncrypted() const
{
    Q_D(const DFMBlockDevice);

    return isEncrypted(d->dbus->path());
}

bool DFMBlockDevice::isLoopDevice() const
{
    Q_D(const DFMBlockDevice);

    return UDisks2::interfaceIsExistes(d->dbus->path(), UDISKS2_SERVICE ".Loop");
}

bool DFMBlockDevice::hasFileSystem(const QString &path)
{
    return UDisks2::interfaceIsExistes(path, UDISKS2_SERVICE ".Filesystem");
}

bool DFMBlockDevice::hasPartition(const QString &path)
{
    return UDisks2::interfaceIsExistes(path, UDISKS2_SERVICE ".Partition");
}

bool DFMBlockDevice::isEncrypted(const QString &path)
{
    return UDisks2::interfaceIsExistes(path, UDISKS2_SERVICE ".Encrypted");
}

QByteArrayList DFMBlockDevice::mountPoints() const
{
    if (!hasFileSystem())
        return QByteArrayList();

    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = ud2.call("Get", UDISKS2_SERVICE ".Filesystem", "MountPoints");

    return qdbus_cast<QByteArrayList>(reply.value());
}

DFMBlockDevice::PTType DFMBlockDevice::ptType() const
{
    Q_D(const DFMBlockDevice);

    if (!UDisks2::interfaceIsExistes(d->dbus->path(), UDISKS2_SERVICE ".PartitionTable")) {
        return InvalidPT;
    }

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = ud2.call("Get", UDISKS2_SERVICE ".PartitionTable", "Type");

    const QString &type = reply.value().toString();

    if (type.isEmpty()) {
        return InvalidPT;
    }

    if (type == "dos") {
        return MBR;
    }

    if (type == "gpt") {
        return GPT;
    }

    return UnknowPT;
}

QList<QPair<QString, QVariantMap> > DFMBlockDevice::childConfiguration() const
{
    Q_D(const DFMBlockDevice);

    if (!isEncrypted()) {
        return QList<QPair<QString, QVariantMap>>();
    }

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = ud2.call("Get", UDISKS2_SERVICE ".Encrypted", "ChildConfiguration");

    return qdbus_cast<QList<QPair<QString, QVariantMap>>>(reply.value());
}

void DFMBlockDevice::setWatchChanges(bool watchChanges)
{
    Q_D(DFMBlockDevice);

    if (d->watchChanges == watchChanges)
        return;

    d->watchChanges = watchChanges;

    OrgFreedesktopDBusObjectManagerInterface *object_manager = UDisks2::objectManager();
    auto sb = QDBusConnection::systemBus();

    if (watchChanges) {
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                this, &DFMBlockDevice::onInterfacesAdded);
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                this, &DFMBlockDevice::onInterfacesRemoved);

        sb.connect(UDISKS2_SERVICE, d->dbus->path(), "org.freedesktop.DBus.Properties",
                   "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
    } else {
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                   this, &DFMBlockDevice::onInterfacesAdded);
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                   this, &DFMBlockDevice::onInterfacesRemoved);

        sb.disconnect(UDISKS2_SERVICE, d->dbus->path(), "org.freedesktop.DBus.Properties",
                      "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
    }
}

void DFMBlockDevice::addConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    d->dbus->AddConfigurationItem(item, options);
}

void DFMBlockDevice::format(const QString &type, const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    d->dbus->Format(type, options);
}

void DFMBlockDevice::format(const DFMBlockDevice::FSType &type, const QVariantMap &options)
{
    if (type < ext2)
        return;

    format(QString::fromLatin1(QMetaEnum::fromType<FSType>().valueToKey(type)), options);
}

QList<QPair<QString, QVariantMap> > DFMBlockDevice::getSecretConfiguration(const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    return d->dbus->GetSecretConfiguration(options);
}

QDBusUnixFileDescriptor DFMBlockDevice::openDevice(const QString &mode, const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    return d->dbus->OpenDevice(mode, options);
}

QDBusUnixFileDescriptor DFMBlockDevice::openForBackup(const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    return d->dbus->OpenForBackup(options);
}

QDBusUnixFileDescriptor DFMBlockDevice::openForBenchmark(const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    return d->dbus->OpenForBenchmark(options);
}

QDBusUnixFileDescriptor DFMBlockDevice::openForRestore(const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    return d->dbus->OpenForRestore(options);
}

void DFMBlockDevice::removeConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    d->dbus->RemoveConfigurationItem(item, options);
}

void DFMBlockDevice::rescan(const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    d->dbus->Rescan(options);
}

void DFMBlockDevice::updateConfigurationItem(const QPair<QString, QVariantMap> &old_item, const QPair<QString, QVariantMap> &new_item, const QVariantMap &options)
{
    Q_D(DFMBlockDevice);

    d->dbus->UpdateConfigurationItem(old_item, new_item, options);
}

QString DFMBlockDevice::mount(const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Filesystem", QDBusConnection::systemBus());
    QDBusReply<QString> reply = ud2.call("Mount", options);

    return reply.value();
}

void DFMBlockDevice::unmount(const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Filesystem", QDBusConnection::systemBus());
    QDBusReply<void> reply = ud2.call("Unmount", options);

    Q_UNUSED(reply)
}

/*!
 * \brief Check if we can set the filesystem label.
 *
 * \return
 */
bool DFMBlockDevice::canSetLabel() const
{
    if (!hasFileSystem()) {
        return false;
    }

    // blumia: Since gvfs can't correctly mount a label-renamed patition
    //         we simply disable rename support if we don't unmount the
    //         patition. Will be add back when we switch to udisks2.
    if (/*fsType() == ntfs && */!mountPoints().isEmpty()) {
        return false;
    }

    return true;
}

/*!
 * \brief Sets the filesystem label.
 *
 * \param options Options (currently unused except for standard options).
 */
void DFMBlockDevice::setLabel(const QString &label, const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Filesystem", QDBusConnection::systemBus());
    QDBusReply<void> reply = ud2.call("SetLabel", label, options);

    Q_UNUSED(reply)
}

void DFMBlockDevice::changePassphrase(const QString &passphrase, const QString &new_passphrase, const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Encrypted", QDBusConnection::systemBus());
    QDBusReply<void> reply = ud2.call("ChangePassphrase", passphrase, new_passphrase, options);

    Q_UNUSED(reply)
}

void DFMBlockDevice::lock(const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Encrypted", QDBusConnection::systemBus());
    QDBusReply<void> reply = ud2.call("Lock", options);

    Q_UNUSED(reply)
}

void DFMBlockDevice::unlock(const QString &passphrase, const QVariantMap &options)
{
    Q_D(const DFMBlockDevice);

    QDBusInterface ud2(UDISKS2_SERVICE, d->dbus->path(), UDISKS2_SERVICE ".Encrypted", QDBusConnection::systemBus());
    QDBusReply<void> reply = ud2.call("Unlock", passphrase, options);

    Q_UNUSED(reply)
}

DFMBlockDevice::DFMBlockDevice(const QString &path, QObject *parent)
    : DFMBlockDevice(*new DFMBlockDevicePrivate(this), path, parent)
{

}

DFMBlockDevice::DFMBlockDevice(DFMBlockDevicePrivate &dd, const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    dd.dbus = new OrgFreedesktopUDisks2BlockInterface(UDISKS2_SERVICE, path, QDBusConnection::systemBus(), this);

    connect(this, &DFMBlockDevice::idTypeChanged, this, &DFMBlockDevice::fsTypeChanged);
}

DFM_END_NAMESPACE
