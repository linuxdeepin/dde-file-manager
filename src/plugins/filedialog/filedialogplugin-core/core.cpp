/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "core.h"
#include "dbus/filedialogmanagerdbus.h"
#include "dbus/filedialogmanager_adaptor.h"
#include "views/filedialog.h"
#include "menus/filedialogmenuscene.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/common/menu/menuservice.h"

#include <QDBusError>
#include <QDBusConnection>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DIALOGCORE_USE_NAMESPACE

bool Core::start()
{
    WindowsService::service()->setCustomWindowCreator([](const QUrl &url) {
        return new FileDialog(url);
    });

    connect(&dpfInstance.listener(), &dpf::Listener::pluginsStarted, this, &Core::onAllPluginsStarted);

    return true;
}

bool Core::registerDialogDBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    QString appName { qApp->applicationName() };
    qInfo() << "Current app is" << appName;
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
        qWarning("Cannot register the \"com.deepin.filemanager.filedialog\" service.\n");
        return false;
    }

    FileDialogManagerDBus *manager = new FileDialogManagerDBus();
    Q_UNUSED(new FiledialogmanagerAdaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject(pathName, manager)) {
        qWarning("Cannot register to the D-Bus object: \"/com/deepin/filemanager/filedialogmanager\"\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

void Core::onAllPluginsStarted()
{
    if (!registerDialogDBus())
        abort();

    MenuService::service()->registerScene(FileDialogMenuCreator::name(), new FileDialogMenuCreator);
    bindScene("WorkspaceMenu");
}

void Core::bindScene(const QString &parentScene)
{
    if (MenuService::service()->contains(parentScene)) {
        MenuService::service()->bind(FileDialogMenuCreator::name(), parentScene);
    } else {
        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
            if (scene == parentScene)
                MenuService::service()->bind(FileDialogMenuCreator::name(), scene);
        },
                Qt::DirectConnection);
    }
}
