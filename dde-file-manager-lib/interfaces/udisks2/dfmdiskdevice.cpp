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
#include "dfmdiskdevice.h"
#include "udisks2_interface.h"

DFM_BEGIN_NAMESPACE

class DFMDiskDevicePrivate
{
public:
    OrgFreedesktopUDisks2DriveInterface *dbus = nullptr;
};

DFMDiskDevice::DFMDiskDevice(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMDiskDevicePrivate())
{
    d_ptr->dbus = new OrgFreedesktopUDisks2DriveInterface(UDISKS2_SERVICE, path, QDBusConnection::systemBus(), this);
}

DFMDiskDevice::~DFMDiskDevice()
{

}

QString DFMDiskDevice::path() const
{
    Q_D(const DFMDiskDevice);

    return d->dbus->path();
}

bool DFMDiskDevice::canPowerOff() const
{
    return d_ptr->dbus->canPowerOff();
}

QVariantMap DFMDiskDevice::configuration() const
{
    return d_ptr->dbus->configuration();
}

QString DFMDiskDevice::connectionBus() const
{
    return d_ptr->dbus->connectionBus();
}

bool DFMDiskDevice::ejectable() const
{
    return d_ptr->dbus->ejectable();
}

QString DFMDiskDevice::id() const
{
    return d_ptr->dbus->id();
}

QString DFMDiskDevice::media() const
{
    return d_ptr->dbus->media();
}

bool DFMDiskDevice::mediaAvailable() const
{
    return d_ptr->dbus->mediaAvailable();
}

bool DFMDiskDevice::mediaChangeDetected() const
{
    return d_ptr->dbus->mediaChangeDetected();
}

QStringList DFMDiskDevice::mediaCompatibility() const
{
    return d_ptr->dbus->mediaCompatibility();
}

bool DFMDiskDevice::mediaRemovable() const
{
    return d_ptr->dbus->mediaRemovable();
}

QString DFMDiskDevice::model() const
{
    return d_ptr->dbus->model();
}

bool DFMDiskDevice::optical() const
{
    return d_ptr->dbus->optical();
}

bool DFMDiskDevice::opticalBlank() const
{
    return d_ptr->dbus->opticalBlank();
}

uint DFMDiskDevice::opticalNumAudioTracks() const
{
    return d_ptr->dbus->opticalNumAudioTracks();
}

uint DFMDiskDevice::opticalNumDataTracks() const
{
    return d_ptr->dbus->opticalNumDataTracks();
}

uint DFMDiskDevice::opticalNumSessions() const
{
    return d_ptr->dbus->opticalNumSessions();
}

uint DFMDiskDevice::opticalNumTracks() const
{
    return d_ptr->dbus->opticalNumTracks();
}

bool DFMDiskDevice::removable() const
{
    return d_ptr->dbus->removable();
}

QString DFMDiskDevice::revision() const
{
    return d_ptr->dbus->revision();
}

int DFMDiskDevice::rotationRate() const
{
    return d_ptr->dbus->rotationRate();
}

QString DFMDiskDevice::seat() const
{
    return d_ptr->dbus->seat();
}

QString DFMDiskDevice::serial() const
{
    return d_ptr->dbus->serial();
}

QString DFMDiskDevice::siblingId() const
{
    return d_ptr->dbus->siblingId();
}

qulonglong DFMDiskDevice::size() const
{
    return d_ptr->dbus->size();
}

QString DFMDiskDevice::sortKey() const
{
    return d_ptr->dbus->sortKey();
}

qulonglong DFMDiskDevice::timeDetected() const
{
    return d_ptr->dbus->timeDetected();
}

qulonglong DFMDiskDevice::timeMediaDetected() const
{
    return d_ptr->dbus->timeMediaDetected();
}

QString DFMDiskDevice::vendor() const
{
    return d_ptr->dbus->vendor();
}

QString DFMDiskDevice::WWN() const
{
    return d_ptr->dbus->wWN();
}

void DFMDiskDevice::eject(const QVariantMap &options)
{
    d_ptr->dbus->Eject(options);
}

void DFMDiskDevice::powerOff(const QVariantMap &options)
{
    d_ptr->dbus->PowerOff(options);
}

void DFMDiskDevice::setConfiguration(const QVariantMap &value, const QVariantMap &options)
{
    d_ptr->dbus->SetConfiguration(value, options);
}

DFM_END_NAMESPACE
