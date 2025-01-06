// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "dbus/filedialogmanagerdbus.h"
#include "filedialogmanageradaptor.h"
#include "views/filedialog.h"
#include "menus/filedialogmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QDBusError>
#include <QDBusConnection>

DFMBASE_USE_NAMESPACE
namespace filedialog_core {
DFM_LOG_REISGER_CATEGORY(DIALOGCORE_NAMESPACE)

bool Core::start()
{
    enterHighPerformanceMode();
    FMWindowsIns.setCustomWindowCreator([](const QUrl &url) {
        return new FileDialog(url);
    });

    connect(dpfListener, &dpf::Listener::pluginsStarted, this, &Core::onAllPluginsStarted);

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

}   // namespace filedialog_core
