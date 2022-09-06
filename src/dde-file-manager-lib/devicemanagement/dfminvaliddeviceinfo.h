// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMINVALIDDEVICEINFO_H
#define DFMINVALIDDEVICEINFO_H

#include "dfmabstractdeviceinterface.h"

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMInvalidDeviceInfo : public DFMAbstractDeviceInterface
{
public:
    DFMInvalidDeviceInfo();

    void mount() override;

    bool unmountable() override;
    void unmount() override;

    bool ejectable() override;
    void eject() override;

    bool isReadOnly() const override;

    QString name() const override;
    bool canRename() const override;
    QString displayName() const override; // device display name.
    QString iconName() const override; // device icon (theme) name.

    bool deviceUsageValid() const override; // storage valid / available or not.
    quint64 availableBytes() const override;
    quint64 freeBytes() const override; // may NOT equals to `total - used`
    quint64 totalBytes() const override;

    QString mountpointPath() const override; // "/media/blumia/xxx"
};

DFM_END_NAMESPACE

#endif // DFMINVALIDDEVICEINFO_H
