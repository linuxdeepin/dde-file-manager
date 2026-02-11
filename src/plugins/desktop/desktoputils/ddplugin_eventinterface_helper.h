// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDPLUGIN_EVENTINTERFACE_HELPER_H
#define DDPLUGIN_EVENTINTERFACE_HELPER_H

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>

#include <dfm-framework/dpf.h>

#define CanvasCorePush(topic) \
    dpfSlotChannel->push("ddplugin_core", QT_STRINGIFY2(topic))

#define CanvasCorePush2(topic, args...) \
    dpfSlotChannel->push("ddplugin_core", QT_STRINGIFY2(topic), ##args)

namespace ddplugin_desktop_util {

static inline QList<DFMBASE_NAMESPACE::ScreenPointer> screenProxyLogicScreens()
{
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_LogicScreens);
    return ret.value<QList<DFMBASE_NAMESPACE::ScreenPointer>>();
}

static inline DFMBASE_NAMESPACE::DisplayMode screenProxyLastChangedMode()
{
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_LastChangedMode);
    return static_cast<DFMBASE_NAMESPACE::DisplayMode>(ret.toInt());
}

static inline QList<DFMBASE_NAMESPACE::ScreenPointer> screenProxyScreens()
{
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_Screens);
    return ret.value<QList<DFMBASE_NAMESPACE::ScreenPointer>>();
}

static inline DFMBASE_NAMESPACE::ScreenPointer screenProxyPrimaryScreen()
{
    const QVariant &ret = CanvasCorePush(slot_ScreenProxy_PrimaryScreen);
    return ret.value<DFMBASE_NAMESPACE::ScreenPointer>();
}

static inline DFMBASE_NAMESPACE::ScreenPointer screenProxyScreen(const QString &screen)
{
    const QVariant &ret = CanvasCorePush2(slot_ScreenProxy_Screen, screen);
    return ret.value<DFMBASE_NAMESPACE::ScreenPointer>();
}

static inline QList<QWidget *> desktopFrameRootWindows()
{
    const QVariant &ret = CanvasCorePush(slot_DesktopFrame_RootWindows);
    return ret.value<QList<QWidget *>>();
}

}

#endif   // DDPLUGIN_EVENTINTERFACE_HELPER_H
