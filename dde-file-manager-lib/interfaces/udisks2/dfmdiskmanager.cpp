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
#include "dfmdiskmanager.h"
#include "udisks2_dbus_common.h"
#include "objectmanager_interface.h"
#include "dfmblockdevice.h"
#include "dfmblockpartition.h"
#include "dfmdiskdevice.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QXmlStreamReader>
#include <QDBusMetaType>
#include <QDebug>

DFM_BEGIN_NAMESPACE

class DFMDiskManagerPrivate
{
public:
    DFMDiskManagerPrivate(DFMDiskManager *qq);

    void updateBlockDeviceMountPointsMap();

    bool watchChanges = false;
    QMap<QString, QByteArrayList> blockDeviceMountPointsMap;

    DFMDiskManager *q_ptr;
};

DFMDiskManagerPrivate::DFMDiskManagerPrivate(DFMDiskManager *qq)
    : q_ptr(qq)
{

}

void DFMDiskManagerPrivate::updateBlockDeviceMountPointsMap()
{
    blockDeviceMountPointsMap.clear();

    auto om = UDisks2::objectManager();
    const QMap<QDBusObjectPath, QMap<QString, QVariantMap>> &objects = om->GetManagedObjects().value();
    auto begin = objects.constBegin();

    while (begin != objects.constEnd()) {
        const QString path = begin.key().path();
        const QMap<QString, QVariantMap> object = begin.value();

        ++begin;

        if (!path.startsWith(QStringLiteral("/org/freedesktop/UDisks2/block_devices/"))) {
            continue;
        }

        const QVariantMap &filesystem = object.value(QStringLiteral(UDISKS2_SERVICE ".Filesystem"));

        if (filesystem.isEmpty()) {
            continue;
        }

        blockDeviceMountPointsMap[path] = qdbus_cast<QByteArrayList>(filesystem.value("MountPoints"));
    }
}

void DFMDiskManager::onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties)
{
    const QString &path = object_path.path();
    const QString &path_drive = QStringLiteral("/org/freedesktop/UDisks2/drives/");
    const QString &path_device = QStringLiteral("/org/freedesktop/UDisks2/block_devices/");

    if (path.startsWith(path_drive)) {
        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Drive"))) {
            Q_EMIT diskDeviceAdded(path);
        }
    } else if (path.startsWith(path_device)) {
        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Block"))) {
            Q_EMIT blockDeviceAdded(path);
        }

        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Filesystem"))) {
            Q_D(DFMDiskManager);

            d->blockDeviceMountPointsMap.remove(object_path.path());

            Q_EMIT fileSystemAdded(path);
        }
    }
}

void DFMDiskManager::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    const QString &path = object_path.path();

    for (const QString &i : interfaces) {
        if (i == QStringLiteral(UDISKS2_SERVICE ".Drive")) {
            Q_EMIT diskDeviceRemoved(path);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Filesystem")) {
            Q_D(DFMDiskManager);

            d->blockDeviceMountPointsMap.remove(object_path.path());

            Q_EMIT fileSystemRemoved(path);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Block")) {
            Q_EMIT blockDeviceRemoved(path);
        }
    }
}

void DFMDiskManager::onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties, const QDBusMessage &message)
{
    Q_D(DFMDiskManager);

    if (interface != UDISKS2_SERVICE ".Filesystem") {
        return;
    }

    if (!changed_properties.contains("MountPoints")) {
        return;
    }

    const QString &path = message.path();
    const QByteArrayList old_mount_points = d->blockDeviceMountPointsMap.value(path);
    const QByteArrayList &new_mount_points = qdbus_cast<QByteArrayList>(changed_properties.value("MountPoints"));

    d->blockDeviceMountPointsMap[path] = new_mount_points;

    Q_EMIT mountPointsChanged(path, old_mount_points, new_mount_points);

    if (old_mount_points.isEmpty()) {
        if (!new_mount_points.isEmpty()) {
            Q_EMIT mountAdded(path, new_mount_points.first());
        }
    } else if (new_mount_points.isEmpty()) {
        Q_EMIT mountRemoved(path, old_mount_points.first());
    }
}

/*!
 * \class DFMDiskManager
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMDiskManager provide severial ways to manage devices and partitions.
 *
 * \sa DFMBlockPartition, DFMBlockDevice, UDiskDeviceInfo
 */

DFMDiskManager::DFMDiskManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMDiskManagerPrivate(this))
{

}

DFMDiskManager::~DFMDiskManager()
{

}

static QStringList getDBusNodeNameList(const QString &service, const QString &path, const QDBusConnection &connection)
{
    QDBusInterface ud2(service, path, "org.freedesktop.DBus.Introspectable", connection);
    QDBusReply<QString> reply = ud2.call("Introspect");
    QXmlStreamReader xml_parser(reply.value());
    QStringList nodeList;

    while (!xml_parser.atEnd()) {
        xml_parser.readNext();

        if (xml_parser.tokenType() == QXmlStreamReader::StartElement
                && xml_parser.name().toString() == "node") {
            const QString &name = xml_parser.attributes().value("name").toString();

            if (!name.isEmpty()) {
                nodeList << path + "/" + name;
            }
        }
    }

    return nodeList;
}

QStringList DFMDiskManager::blockDevices() const
{
    return getDBusNodeNameList(UDISKS2_SERVICE, "/org/freedesktop/UDisks2/block_devices", QDBusConnection::systemBus());
}

QStringList DFMDiskManager::diskDevices() const
{
    return getDBusNodeNameList(UDISKS2_SERVICE, "/org/freedesktop/UDisks2/drives", QDBusConnection::systemBus());
}

bool DFMDiskManager::watchChanges() const
{
    Q_D(const DFMDiskManager);

    return d->watchChanges;
}

QString DFMDiskManager::objectPrintable(const QObject *object)
{
    QString string;
    QDebug debug(&string);
    const QMetaObject *mo = object->metaObject();

    debug << object;

    int property_count = mo->propertyCount();
    int base_property_count = QObject::staticMetaObject.propertyCount();

    debug << "\n";

    for (int i = base_property_count; i < property_count; ++i) {
        const QMetaProperty &mp = mo->property(i);

        debug.nospace() << mp.name() << ": " << mp.read(object);
        debug << "\n";
    }

    return string;
}

DFMBlockDevice *DFMDiskManager::createBlockDevice(const QString &path, QObject *parent)
{
    return new DFMBlockDevice(path, parent);
}

DFMBlockPartition *DFMDiskManager::createBlockPartition(const QString &path, QObject *parent)
{
    return new DFMBlockPartition(path, parent);
}

DFMDiskDevice *DFMDiskManager::createDiskDevice(const QString &path, QObject *parent)
{
    return new DFMDiskDevice(path, parent);
}

QDBusError DFMDiskManager::lastError()
{
    return QDBusConnection::systemBus().lastError();
}

void DFMDiskManager::setWatchChanges(bool watchChanges)
{
    Q_D(DFMDiskManager);

    if (d->watchChanges == watchChanges)
        return;

    OrgFreedesktopDBusObjectManagerInterface *object_manager = UDisks2::objectManager();
    auto sc = QDBusConnection::systemBus();

    if (watchChanges) {
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                this, &DFMDiskManager::onInterfacesAdded);
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                this, &DFMDiskManager::onInterfacesRemoved);

        d->updateBlockDeviceMountPointsMap();

        sc.connect(UDISKS2_SERVICE, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage&)));
    } else {
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                   this, &DFMDiskManager::onInterfacesAdded);
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                   this, &DFMDiskManager::onInterfacesRemoved);

        d->blockDeviceMountPointsMap.clear();

        sc.disconnect(UDISKS2_SERVICE, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                      this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage&)));
    }
}

DFM_END_NAMESPACE
