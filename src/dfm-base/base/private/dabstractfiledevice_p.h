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
#ifndef DABSTRACTFILEDEVICE_P_H
#define DABSTRACTFILEDEVICE_P_H

#include "base/dabstractfiledevice.h"
#include "dfm-base/dfm_base_global.h"

#include <QUrl>

class DAbstractFileDevicePrivate
{
public:
    explicit DAbstractFileDevicePrivate(DAbstractFileDevice *qq);

    virtual ~DAbstractFileDevicePrivate();

    DAbstractFileDevice *q_ptr;
    QUrl url;

    Q_DECLARE_PUBLIC(DAbstractFileDevice)
};

#endif // DABSTRACTFILEDEVICE_P_H
