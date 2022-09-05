// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLOCALFILEDEVICE_P_H
#define DLOCALFILEDEVICE_P_H

#include "dfileiodeviceproxy_p.h"

DFM_BEGIN_NAMESPACE

class DLocalFileDevicePrivate : public DFileIODeviceProxyPrivate
{
public:
    explicit DLocalFileDevicePrivate(dde_file_manager::DLocalFileDevice *qq);

    ~DLocalFileDevicePrivate() override;

    QPointer<QFile> file = nullptr;

    Q_DECLARE_PUBLIC(DLocalFileDevice)
};

DFM_END_NAMESPACE

#endif // DLOCALFILEDEVICE_P_H
