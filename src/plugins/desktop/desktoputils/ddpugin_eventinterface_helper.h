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
#ifndef DDPUGIN_EVENTINTERFACE_HELPER_H
#define DDPUGIN_EVENTINTERFACE_HELPER_H

#include "interfaces/screen/abstractscreenproxy.h"

#include <dfm-framework/dpf.h>

#define CanvasCorePush(topic) \
        dpfSlotChannel->push("ddplugin_core", QT_STRINGIFY2(topic))

#define CanvasCorePush2(topic, args...) \
        dpfSlotChannel->push("ddplugin_core", QT_STRINGIFY2(topic), ##args)

namespace ddplugin_desktop_util {

static inline QVector<DFMBASE_NAMESPACE::ScreenPointer> screenProxyLogicScreens() {
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_LogicScreens);
    return ret.value<QVector<DFMBASE_NAMESPACE::ScreenPointer>>();
}

static inline DFMBASE_NAMESPACE::DisplayMode screenProxyLastChangedMode() {
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_LastChangedMode);
    return static_cast<DFMBASE_NAMESPACE::DisplayMode>(ret.toInt());
}

static inline QVector<DFMBASE_NAMESPACE::ScreenPointer> screenProxyScreens() {
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_Screens);
    return ret.value<QVector<DFMBASE_NAMESPACE::ScreenPointer>>();
}

static inline DFMBASE_NAMESPACE::ScreenPointer screenProxyPrimaryScreen() {
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_PrimaryScreen);
    return ret.value<DFMBASE_NAMESPACE::ScreenPointer>();
}

static inline DFMBASE_NAMESPACE::ScreenPointer screenProxyScreen(const QString &screen) {
    const QVariant &ret = CanvasCorePush2(slot_ScreenProxy_Screen, screen);
    return ret.value<DFMBASE_NAMESPACE::ScreenPointer>();
}

static inline QList<QWidget *> desktopFrameRootWindows() {
    const QVariant &ret = CanvasCorePush(slot_DesktopFrame_RootWindows);
    return ret.value<QList<QWidget *>>();
}

}

#endif // DDPUGIN_EVENTINTERFACE_HELPER_H
