// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "frame/windowframe.h"
#include "screen/screenproxyqt.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <dfm-io/dfmio_utils.h>

#include <QDBusInterface>
#include <QDBusPendingCall>

Q_DECLARE_METATYPE(QStringList *)

DFMBASE_USE_NAMESPACE

#define CanvasCorePublish(topic) \
    dpfSignalDispatcher->publish(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic))

#define CanvasCoreSlot(topic, args...) \
    dpfSlotChannel->connect(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasCoreDisconnect(topic) \
    dpfSlotChannel->disconnect(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic))

#define CanvasCorelFollow(topic, args...) \
    dpfHookSequence->follow(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasCorelUnfollow(topic, args...) \
    dpfHookSequence->unfollow(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

namespace ddplugin_core {
DFM_LOG_REISGER_CATEGORY(DDPCORE_NAMESPACE)

static void registerFileSystem()
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

void Core::initialize()
{
    registerFileSystem();
    connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &Core::onStart);
    dpfSignalDispatcher->subscribe(GlobalEventType::kLoadPlugins,
                                   this, &Core::handleLoadPlugins);

    // NOTE(xust): this is used to launch GVolumeMonitor in main thread, this function obtained the GVolumeMonitor instance indirectly,
    // a GVolumeMonitor instance must run in main thread to make sure the messages about device change can be send correctly,
    // the GVolumeMonitor instance is expected to be initialized in DDeviceManager but the DDeviceManager is delay intialized when
    // main window of desktop showed. So invoke the method here to make sure the instance is initialized in main thread.
    // NOTE(xust): this may take 10ms when app launch, but no better way to solve the level-2 issue for now.
    DFMIO::DFMUtils::fileIsRemovable(QUrl::fromLocalFile("/"));

    QString err;
    DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.desktop", &err);
    fmInfo() << "register desktop dconfig:" << err;
}

bool Core::start()
{
    connectToServer();

    // 手动初始化application
    app = new DFMBASE_NAMESPACE::Application();

    handle = new EventHandle();
    return handle->init();
}

void Core::stop()
{
    delete handle;
    handle = nullptr;

    delete app;
    app = nullptr;
}

void Core::onStart()
{
    connect(handle->frame, &WindowFrame::windowShowed, this, &Core::onFrameReady);
    // create desktop frame windows.
    handle->frame->buildBaseWindow();
}

void Core::onFrameReady()
{
    disconnect(handle->frame, &WindowFrame::windowShowed, this, &Core::onFrameReady);
    // no window to show, load plugins directly.
    if (handle->frame->rootWindows().isEmpty()) {
        QMetaObject::invokeMethod(this, "initializeAfterPainted", Qt::QueuedConnection);
    } else {
        // to get paint event.
        qApp->installEventFilter(this);
    }
}

void Core::handleLoadPlugins(const QStringList &names)
{
    std::for_each(names.begin(), names.end(), [](const QString &name) {
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        fmInfo() << "About to load plugin:" << name;
        auto plugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj(name) };
        if (plugin) {
            bool ret = DPF_NAMESPACE::LifeCycle::loadPlugin(plugin);
            if (ret)
                fmInfo() << "lazy load State: " << plugin->pluginState();
            else
                fmCritical() << "fail to load plugin: " << plugin->pluginState();
        }
    });
}

bool Core::eventFilter(QObject *watched, QEvent *event)
{
    // windows paint
    if (event->type() == QEvent::Paint) {
        fmInfo() << "one window painting" << watched;
        qApp->removeEventFilter(this);
        QMetaObject::invokeMethod(this, "initializeAfterPainted", Qt::QueuedConnection);
    }
    return false;
}

void Core::connectToServer()
{
    if (!DevProxyMng->initService()) {
        fmCritical() << "device manager cannot connect to server!";
        DevMngIns->startMonitor();
    }

    fmInfo() << "connectToServer finished";
}

void Core::initializeAfterPainted()
{
    std::call_once(lazyFlag, []() {
        // init all lazy plguis call once
        const QStringList &list { DPF_NAMESPACE::LifeCycle::lazyLoadList() };
        fmInfo() << "load lazy plugins" << list;
        dpfSignalDispatcher->publish(GlobalEventType::kLoadPlugins, list);
        // init clipboard
        ClipBoard::instance()->readFirstClipboard();
    });
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

    CanvasCorelUnfollow(hook_ScreenProxy_ScreensInUse, &EventHandle::screensInUse);

    delete frame;
    frame = nullptr;

    delete screenProxy;
    screenProxy = nullptr;
}

bool EventHandle::init()
{
    screenProxy = new ScreenProxyQt();
    screenProxy->reset();

    // send signal event
    connect(screenProxy, &AbstractScreenProxy::screenChanged, this, &EventHandle::publishScreenChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::displayModeChanged, this, &EventHandle::publishDisplayModeChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::screenGeometryChanged, this, &EventHandle::publishScreenGeometryChanged, Qt::DirectConnection);
    connect(screenProxy, &AbstractScreenProxy::screenAvailableGeometryChanged, this, &EventHandle::publishScreenAvailableGeometryChanged, Qt::DirectConnection);

    // screen slot event
    // CanvasCoreSlot(slot_ScreenProxy_Instance, &EventHandle::screenProxyInstance);
    CanvasCoreSlot(slot_ScreenProxy_PrimaryScreen, &EventHandle::primaryScreen);
    CanvasCoreSlot(slot_ScreenProxy_Screens, &EventHandle::screens);
    CanvasCoreSlot(slot_ScreenProxy_LogicScreens, &EventHandle::logicScreens);
    CanvasCoreSlot(slot_ScreenProxy_Screen, &EventHandle::screen);
    CanvasCoreSlot(slot_ScreenProxy_DevicePixelRatio, &EventHandle::devicePixelRatio);
    CanvasCoreSlot(slot_ScreenProxy_DisplayMode, &EventHandle::displayMode);
    CanvasCoreSlot(slot_ScreenProxy_LastChangedMode, &EventHandle::lastChangedMode);
    CanvasCoreSlot(slot_ScreenProxy_Reset, &EventHandle::reset);

    // return screens in use.
    CanvasCorelFollow(hook_ScreenProxy_ScreensInUse, &EventHandle::screensInUse);

    frame = new WindowFrame();
    frame->init();

    // desktop frame sigal
    // all the signals are non interruptible, so using Qt::DirectConnection.
    connect(frame, &AbstractDesktopFrame::windowAboutToBeBuilded, this, &EventHandle::publishWindowAboutToBeBuilded, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::windowBuilded, this, &EventHandle::publishWindowBuilded, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::windowShowed, this, &EventHandle::publishWindowShowed, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::geometryChanged, this, &EventHandle::publishGeometryChanged, Qt::DirectConnection);
    connect(frame, &AbstractDesktopFrame::availableGeometryChanged, this, &EventHandle::publishAvailableGeometryChanged, Qt::DirectConnection);

    // CanvasCoreSlot(slot_DesktopFrame_Instance, &EventHandle::desktopFrame);
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

QList<ScreenPointer> EventHandle::screens()
{
    return screenProxy->screens();
}

QList<ScreenPointer> EventHandle::logicScreens()
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

bool EventHandle::screensInUse(QStringList *out)
{
    if (out)
        *out = frame->bindedScreens();

    return false;
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

}   // namespace ddplugin_core
