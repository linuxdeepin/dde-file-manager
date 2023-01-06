// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfminvaliddeviceinfo.h"

#include <QIcon>

DFM_BEGIN_NAMESPACE

DFMInvalidDeviceInfo::DFMInvalidDeviceInfo()
{

}

void DFMInvalidDeviceInfo::mount()
{

}

bool DFMInvalidDeviceInfo::unmountable()
{
    return false;
}

void DFMInvalidDeviceInfo::unmount()
{

}

bool DFMInvalidDeviceInfo::ejectable()
{
    return false;
}

void DFMInvalidDeviceInfo::eject()
{

}

bool DFMInvalidDeviceInfo::isReadOnly() const
{
    return true;
}

QString DFMInvalidDeviceInfo::name() const
{
    return QStringLiteral("Invalid Device");
}

bool DFMInvalidDeviceInfo::canRename() const
{
    return false;
}

QString DFMInvalidDeviceInfo::displayName() const
{
    return QStringLiteral("Invalid Device");
}

QString DFMInvalidDeviceInfo::iconName() const
{
    return "dialog-error";
}

bool DFMInvalidDeviceInfo::deviceUsageValid() const
{
    return false;
}

quint64 DFMInvalidDeviceInfo::availableBytes() const
{
    return -1;
}

quint64 DFMInvalidDeviceInfo::freeBytes() const
{
    return -1;
}

quint64 DFMInvalidDeviceInfo::totalBytes() const
{
    return 0;
}

QString DFMInvalidDeviceInfo::mountpointPath() const
{
    return QString();
}

DFM_END_NAMESPACE
