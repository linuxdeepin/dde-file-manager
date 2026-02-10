// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "dbus/filedialogmanagerdbus.h"
#include "filedialogmanageradaptor.h"
#include "views/filedialog.h"
#include "menus/filedialogmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QDBusError>
#include <QDBusConnection>

DFMBASE_USE_NAMESPACE
namespace filedialog_core {
DFM_LOG_REGISTER_CATEGORY(DIALOGCORE_NAMESPACE)

bool Core::start()
{
    enterHighPerformanceMode();
    FMWindowsIns.setCustomWindowCreator([](const QUrl &url) {
        return new FileDialog(url);
    });

    connect(dpfListener, &dpf::Listener::pluginsStarted, this, &Core::onAllPluginsStarted);

    bool connected = QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          "PrepareForShutdown",
                                                          this,
                                                          SLOT(exitOnShutdown(bool)));
    fmDebug() << "login1::PrepareForShutdown connected:" << connected;

    return true;
}

bool Core::registerDialogDBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        fmWarning("File Dialog: Cannot connect to the D-Bus session bus.");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    QString appName { qApp->applicationName() };
    fmInfo() << "Current app is" << appName;
    QString serviceName { "com.deepin.filemanager.filedialog" };
    QString pathName { "/com/deepin/filemanager/filedialogmanager" };

    if (appName.endsWith("x11")) {
        serviceName += "_x11";
        pathName += "_x11";
    } else if (appName.endsWith("wayland")) {
        serviceName += "_wayland";
        pathName += "_wayland";
    }

    if (!QDBusConnection::sessionBus().registerService(serviceName)) {
        fmWarning("File Dialog: Cannot register the \"com.deepin.filemanager.filedialog\" service.\n");
        return false;
    }

    FileDialogManagerDBus *manager = new FileDialogManagerDBus();
    Q_UNUSED(new FiledialogmanagerAdaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject(pathName, manager)) {
        fmWarning("File Dialog: Cannot register to the D-Bus object: \"/com/deepin/filemanager/filedialogmanager\"\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

void Core::onAllPluginsStarted()
{
    if (!registerDialogDBus())
        abort();

    DFMBASE_NAMESPACE::Application::instance()->appSetting()->setReadOnly(true);
    DFMBASE_NAMESPACE::Application::instance()->appObtuselySetting()->setReadOnly(true);

    dfmplugin_menu_util::menuSceneRegisterScene(FileDialogMenuCreator::name(), new FileDialogMenuCreator);
    bindScene("WorkspaceMenu");
}

void Core::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(FileDialogMenuCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Core::bindSceneOnAdded);
    }
}

void Core::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Core::bindSceneOnAdded);
        bindScene(newScene);
    }
}

void Core::enterHighPerformanceMode()
{
    auto systemBusIFace = QDBusConnection::systemBus().interface();
    if (!systemBusIFace) {
        fmWarning() << "systemBus is not available.";
        return;
    }

    if (!systemBusIFace->isServiceRegistered("com.deepin.system.Power")) {
        fmWarning() << "com.deepin.system.Power is not registered";
        return;
    }

    fmInfo() << "About to call dbus LockCpuFreq";
    QDBusInterface daemonIface("com.deepin.system.Power", "/com/deepin/system/Power", "com.deepin.system.Power",
                               QDBusConnection::systemBus());
    // Pull up the CPU frequency for 3s
    daemonIface.asyncCall("LockCpuFreq", "performance", 3);
}

void Core::exitOnShutdown(bool shutdown)
{
    if (shutdown) {
        fmInfo() << "PrepareForShutdown is emitted, exit...";
        // 设置一个5秒的看门狗定时器。
        // 如果5秒后我们还“活着”，就强制退出。
        const int watchdogTimeout = 5000;
        QTimer::singleShot(watchdogTimeout, [=]() {
            // 如果这段代码被执行，说明优雅退出失败了。
            // 记录一条日志是至关重要的，这样你就知道是看门狗触发了退出。
            fmWarning() << "Graceful shutdown timed out after" << watchdogTimeout << "ms. Forcing exit with _Exit(0).";

            // 立即终止进程
            ::_Exit(0);
        });

        // 尝试正常、优雅地退出
        qApp->quit();
    }
}

}   // namespace filedialog_core
