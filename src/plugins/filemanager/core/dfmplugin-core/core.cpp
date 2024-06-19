// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "events/coreeventreceiver.h"
#include "utils/corehelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-gui/panel.h>
#include <dfm-gui/windowmanager.h>

#include <dfm-mount/ddevicemanager.h>

#include <QTimer>
#include <QApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QQuickWindow>

#include <DMainWindow>
#include <DApplication>

DFMBASE_USE_NAMESPACE
DFMGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(const char *)
namespace dfmplugin_core {
namespace GlobalPrivate {
static Application *kDFMApp { nullptr };
}   // namespace GlobalPrivate
DFM_LOG_REISGER_CATEGORY(DPCORE_NAMESPACE)

#define DAPP_ATT_WIDGETS 14
#define DAPP_USING_ATT(VAL, dappVal) \
    (DApplication::setAttribute(static_cast<Qt::ApplicationAttribute>(VAL), dappVal), "use dapp att")

void Core::initialize()
{
    QString errStr;

    // 注册路由
    UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

    connect(dpfListener, &dpf::Listener::pluginsInitialized, this, &Core::onAllPluginsInitialized);
    connect(dpfListener, &dpf::Listener::pluginsStarted, this, &Core::onAllPluginsStarted);
    connect(FMQuickWindowIns, &WindowManager::windowOpened, this, [this](quint64 id) {
        QTimer::singleShot(0, [this, id]() {
            onWindowOpened(id);
        });
    });
}

bool Core::start()
{
    GlobalPrivate::kDFMApp = new Application;   // must create it

    connectToServer();

    // the object must be initialized in main thread, otherwise the GVolumeMonitor do not have an event loop.
    static std::once_flag flg;
    std::call_once(flg, [this] {
        QTimer::singleShot(500, this, [] { DFMMOUNT::DDeviceManager::instance(); });
    });

    return true;
}

void Core::stop()
{
    if (GlobalPrivate::kDFMApp)
        delete GlobalPrivate::kDFMApp;
}

void Core::connectToServer()
{
    // mount business
    if (!DevProxyMng->initService()) {
        // active server
        QDBusInterface ifs("org.deepin.filemanager.server",
                           "/org/deepin/filemanager/server");
        ifs.asyncCall("Ping");

        fmCritical() << "device manager cannot connect to service!";
        DevMngIns->startMonitor();
        DevMngIns->startPollingDeviceUsage();
        DevMngIns->enableBlockAutoMount();
    }
}

void Core::onAllPluginsInitialized()
{
    fmInfo() << "All plugins initialized";
    // createWindow may return an existing window, which does not need to be processed again aboutToOpen
    connect(FMQuickWindowIns, &WindowManager::windowCreated, this, [](quint64 id) {
        auto handle { FMQuickWindowIns->findWindowById(id) };
        if (handle)
            handle->window()->installEventFilter(&CoreHelper::instance());
    });
    // subscribe events
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl,
                                   CoreEventReceiver::instance(), &CoreEventReceiver::handleChangeUrl);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewWindow,
                                   CoreEventReceiver::instance(), static_cast<void (CoreEventReceiver::*)(const QUrl &)>(&CoreEventReceiver::handleOpenWindow));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewWindow,
                                   CoreEventReceiver::instance(), static_cast<void (CoreEventReceiver::*)(const QUrl &, const QVariant &)>(&CoreEventReceiver::handleOpenWindow));
    dpfSignalDispatcher->subscribe(GlobalEventType::kLoadPlugins,
                                   CoreEventReceiver::instance(), &CoreEventReceiver::handleLoadPlugins);
    dpfSignalDispatcher->subscribe(GlobalEventType::kHeadlessStarted,
                                   CoreEventReceiver::instance(), &CoreEventReceiver::handleHeadless);
    dpfSignalDispatcher->subscribe(GlobalEventType::kShowSettingDialog, CoreEventReceiver::instance(),
                                   &CoreEventReceiver::handleShowSettingDialog);
}

void Core::onAllPluginsStarted()
{
    fmInfo() << "All plugins started";
    // dde-select-dialog also uses the core plugin, don't start filemanger window
    QString &&curAppName { qApp->applicationName() };
    if (!WindowUtils::isWayLand())
        qDebug() << "start" << DAPP_USING_ATT(DAPP_ATT_WIDGETS, 122);
    if (curAppName == "dde-file-manager")
        dpfSignalDispatcher->publish(DPF_MACRO_TO_STR(DPCORE_NAMESPACE), "signal_StartApp");
    else
        fmInfo() << "Current app name is: " << curAppName << " Don't show filemanger window";
}

void Core::onWindowOpened(quint64 winid)
{
    Q_UNUSED(winid)

    static std::once_flag flag;
    std::call_once(flag, []() {
        QTimer::singleShot(200, []() {
            // init all lazy plguis call once
            const QStringList &list { DPF_NAMESPACE::LifeCycle::lazyLoadList() };
            dpfSignalDispatcher->publish(GlobalEventType::kLoadPlugins, list);
            qDebug() << "end" << DAPP_USING_ATT(DAPP_ATT_WIDGETS, 0);
        });

        // TODO: freeze 5s, fix it!
        QTimer::singleShot(2000, []() {
            // init clipboard
            ClipBoard::instance()->onClipboardDataChanged();
        });
    });
}
}   // namespace dfmplugin_core
