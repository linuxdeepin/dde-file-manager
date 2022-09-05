// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEDEVICE_P_H
#define DFILEDEVICE_P_H

#include "dfmglobal.h"
#include "dfiledevice.h"

DFM_BEGIN_NAMESPACE

class DFileDevicePrivate
{
public:
    explicit DFileDevicePrivate(DFileDevice *qq);
    virtual ~DFileDevicePrivate();

    DFileDevice *q_ptr;
    DUrl url;

    Q_DECLARE_PUBLIC(DFileDevice)
};

DFM_END_NAMESPACE

#endif // DFILEDEVICE_P_H
