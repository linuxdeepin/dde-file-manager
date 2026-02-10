// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewsettingutil.h"

#include <DApplication>

#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_canvas;

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
