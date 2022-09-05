// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEIODEVICEPROXY_P_H
#define DFILEIODEVICEPROXY_P_H

#include "dfileiodeviceproxy.h"
#include "dfiledevice_p.h"

#include <QPointer>

DFM_BEGIN_NAMESPACE
class DFileIODeviceProxyPrivate : public DFileDevicePrivate
{
public:
    explicit DFileIODeviceProxyPrivate(DFileIODeviceProxy *qq);

    QPointer<QIODevice> device = nullptr;
};
DFM_END_NAMESPACE

#endif // DFILEIODEVICEPROXY_P_H
