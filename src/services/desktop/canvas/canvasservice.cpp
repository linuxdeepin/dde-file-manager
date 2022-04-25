/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "private/canvasservice_p.h"

DSB_D_USE_NAMESPACE

CanvasServicePrivate::CanvasServicePrivate(CanvasService *parent)
    : QObject(parent)
    , q(parent)
{
    setObjectName("dfm_service_desktop::CanvasServicePrivate");
}

CanvasService::CanvasService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<CanvasService>()
    , EventInformant()
    , d(new CanvasServicePrivate(this))

{
}

CanvasService::~CanvasService()
{

}

void CanvasService::notify(int eventType, const QStringList &eventKeys) const
{
    if (!eventKeys.isEmpty()) {
        EventInformant::notify(eventType, eventKeys);
        emit sigEventChanged(eventType, eventKeys);
    }
}


