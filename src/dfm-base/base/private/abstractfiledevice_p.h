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
#ifndef ABSTRACTFILEDEVICE_P_H
#define ABSTRACTFILEDEVICE_P_H

#include "base/abstractfiledevice.h"
#include "dfm-base/dfm_base_global.h"

#include <QUrl>

class AbstractFileDevicePrivate
{
public:
    explicit AbstractFileDevicePrivate(AbstractFileDevice *qq);
    virtual ~AbstractFileDevicePrivate();

    AbstractFileDevice *q_ptr;
    QUrl url; // 文件的url

    Q_DECLARE_PUBLIC(AbstractFileDevice)
};

#endif // ABSTRACTFILEDEVICE_P_H
