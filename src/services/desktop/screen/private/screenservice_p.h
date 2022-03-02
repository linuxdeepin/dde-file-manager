/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef SCREENSERVICE_P_H
#define SCREENSERVICE_P_H

#include "dd_service_global.h"
#include "screen/screenservice.h"

#include <interfaces/screen/abstractscreenproxy.h>

DSB_D_BEGIN_NAMESPACE

class ScreenServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit ScreenServicePrivate(ScreenService *parent);
public slots:
    bool setProxy(QObject *);
public:
    DFMBASE_NAMESPACE::AbstractScreenProxy *proxy = nullptr;
private:
    ScreenService *q;
};

DSB_D_END_NAMESPACE

#endif // SCREENSERVICE_P_H
