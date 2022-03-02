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
#include "viewsettingutil.h"

#include <DApplication>

#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

ViewSettingUtil::ViewSettingUtil(QObject *parent) : QObject(parent)
{
    touchDragTimer.setSingleShot(true);
    touchDragTimer.setTimerType(Qt::PreciseTimer);

}

void ViewSettingUtil::checkTouchDrag(QMouseEvent *event)
{
    if (!event)
        return;

    // delay 200ms to enable start drag on touch screen.
    // When the event source is mouseeventsynthesizedbyqt, it is considered that this event is converted from touchbegin
    if ((event->source() == Qt::MouseEventSynthesizedByQt) && (event->button() == Qt::LeftButton)) {
        // Read the pressing duration of DDE configuration
        QObject *themeSettings = reinterpret_cast<QObject *>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
        QVariant touchFlickBeginMoveDelay;
        if (themeSettings)
            touchFlickBeginMoveDelay = themeSettings->property("touchFlickBeginMoveDelay");

        //若dde配置了则使用dde的配置，若没有则使用默认的200ms
        touchDragTimer.setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt() : 200);
        touchDragTimer.start();
    } else {
        touchDragTimer.stop();
    }
}

bool ViewSettingUtil::isDelayDrag() const
{
    return touchDragTimer.isActive();
}
