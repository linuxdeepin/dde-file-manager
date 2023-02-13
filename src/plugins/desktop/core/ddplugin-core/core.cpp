// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "frame/windowframe.h"
#include "screen/screenproxyqt.h"
#include "screen/screenproxydbus.h"

#include "dfm-base/utils/windowutils.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/desktopfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"

DFMBASE_USE_NAMESPACE
DDPCORE_USE_NAMESPACE

#define CanvasCorePublish(topic) \
    dpfSignalDispatcher->publish(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic))

#define CanvasCoreSlot(topic, args...) \
    dpfSlotChannel->connect(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasCoreDisconnect(topic) \
    dpfSlotChannel->disconnect(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic))

static void registerFileSystem()
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    InfoFactory::regClass<LocalFileInfo>(Global::Scheme::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

void ddplugin_core::Core::initialize()
{
    registerFileSystem();
    connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &Core::onStart);
}

bool ddplugin_core::Core::start()
{
    // 手动初始化application
    app = new DFMBASE_NAMESPACE::Application();

    handle = new EventHandle();
    return handle->init();
}

void ddplugin_core::Core::stop()
{
    delete handle;
    handle = nullptr;

    delete app;
    app = nullptr;
}

void Core::onStart()
{
    // create desktop frame windows.
    handle->frame->buildBaseWindow();
}

EventHandle::EventHandle(QObject *parent)
    : QObject(parent)
{
}

EventHandle::~EventHandle()
{
    CanvasCoreDisconnect(slot_ScreenProxy_PrimaryScreen);
    CanvasCoreDisconnect(slot_ScreenProxy_Screens);
    CanvasCoreDisconnect(slot_ScreenProxy_LogicScreens);
    CanvasCoreDisconnect(slot_ScreenProxy_Screen);
    CanvasCoreDisconnect(slot_ScreenProxy_DevicePixelRatio);
    CanvasCoreDisconnect(slot_ScreenProxy_DisplayMode);
    CanvasCoreDisconnect(slot_ScreenProxy_LastChangedMode);
    CanvasCoreDisconnect(slot_ScreenProxy_Reset);

    CanvasCoreDisconnect(slot_DesktopFrame_RootWindows);
    CanvasCoreDisconnect(slot_DesktopFrame_LayoutWidget);

    delete frame;
    frame = nullptr;
}

bool EventHandle::init()
{
    if (WindowUtils::isWayLand())
        screenProxy = new ScreenProxyDBus();
    else
        screenProxy = new ScreenProxyQt();

    screenProxy->reset();

    // send signal event
    connect(screenProxy, &AbstractScreenProxy::screenChanged, this, &EventHandle::publishScreenChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::displayModeChanged, this, &EventHandle::publishDisplayModeChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::screenGeometryChanged, this, &EventHandle::publishScreenGeometryChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::screenAvailableGeometryChanged, this, &EventHandle::publishScreenAvailableGeometryChanged, Qt::DirectConnection);

    // screen slot event
    //CanvasCoreSlot(slot_ScreenProxy_Instance, &EventHandle::screenProxyInstance);
    CanvasCoreSlot(slot_ScreenProxy_PrimaryScreen, &EventHandle::primaryScreen);
    CanvasCoreSlot(slot_ScreenProxy_Screens, &EventHandle::screens);
    CanvasCoreSlot(slot_ScreenProxy_LogicScreens, &EventHandle::logicScreens);
    CanvasCoreSlot(slot_ScreenProxy_Screen, &EventHandle::screen);
    CanvasCoreSlot(slot_ScreenProxy_DevicePixelRatio, &EventHandle::devicePixelRatio);
    CanvasCoreSlot(slot_ScreenProxy_DisplayMode, &EventHandle::displayMode);
    CanvasCoreSlot(slot_ScreenProxy_LastChangedMode, &EventHandle::lastChangedMode);
    CanvasCoreSlot(slot_ScreenProxy_Reset, &EventHandle::reset);

    frame = new WindowFrame();
    frame->init();

    // desktop frame sigal
    // all the signals are non interruptible, so using Qt::DirectConnection.
    connect(frame, &AbstractDesktopFrame::windowAboutToBeBuilded, this, &EventHandle::publishWindowAboutToBeBuilded, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::windowBuilded, this, &EventHandle::publishWindowBuilded, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::windowShowed, this, &EventHandle::publishWindowShowed, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::geometryChanged, this, &EventHandle::publishGeometryChanged, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::availableGeometryChanged, this, &EventHandle::publishAvailableGeometryChanged, Qt::DirectConnection);

    //CanvasCoreSlot(slot_DesktopFrame_Instance, &EventHandle::desktopFrame);
    CanvasCoreSlot(slot_DesktopFrame_RootWindows, &EventHandle::rootWindows);
    CanvasCoreSlot(slot_DesktopFrame_LayoutWidget, &EventHandle::layoutWidget);

    return true;
}

AbstractScreenProxy *EventHandle::screenProxyInstance()
{
    return screenProxy;
}

ScreenPointer EventHandle::primaryScreen()
{
    return screenProxy->primaryScreen();
}

QVector<ScreenPointer> EventHandle::screens()
{
    return screenProxy->screens();
}

QVector<ScreenPointer> EventHandle::logicScreens()
{
    return screenProxy->logicScreens();
}

ScreenPointer EventHandle::screen(const QString &name)
{
    return screenProxy->screen(name);
}

qreal EventHandle::devicePixelRatio()
{
    return screenProxy->devicePixelRatio();
}

int EventHandle::displayMode()
{
    return screenProxy->displayMode();
}

int EventHandle::lastChangedMode()
{
    return screenProxy->lastChangedMode();
}

void EventHandle::reset()
{
    screenProxy->reset();
}

AbstractDesktopFrame *EventHandle::desktopFrame()
{
    return frame;
}

QList<QWidget *> EventHandle::rootWindows()
{
    return frame->rootWindows();
}

void EventHandle::layoutWidget()
{
    frame->layoutChildren();
}

void EventHandle::publishScreenChanged()
{
    CanvasCorePublish(signal_ScreenProxy_ScreenChanged);
}

void EventHandle::publishDisplayModeChanged()
{
    CanvasCorePublish(signal_ScreenProxy_DisplayModeChanged);
}

void EventHandle::publishScreenGeometryChanged()
{
    CanvasCorePublish(signal_ScreenProxy_ScreenGeometryChanged);
}

void EventHandle::publishScreenAvailableGeometryChanged()
{
    CanvasCorePublish(signal_ScreenProxy_ScreenAvailableGeometryChanged);
}

void EventHandle::publishWindowAboutToBeBuilded()
{
    CanvasCorePublish(signal_DesktopFrame_WindowAboutToBeBuilded);
}

void EventHandle::publishWindowBuilded()
{
    CanvasCorePublish(signal_DesktopFrame_WindowBuilded);
}

void EventHandle::publishWindowShowed()
{
    CanvasCorePublish(signal_DesktopFrame_WindowShowed);
}

void EventHandle::publishGeometryChanged()
{
    CanvasCorePublish(signal_DesktopFrame_GeometryChanged);
}

void EventHandle::publishAvailableGeometryChanged()
{
    CanvasCorePublish(signal_DesktopFrame_AvailableGeometryChanged);
}
