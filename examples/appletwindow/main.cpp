// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlEngine>

#include <dfm-gui/appletmanager.h>
#include <dfm-gui/windowmanager.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    dpf::PluginQuickMetaDataCreator creator;
    creator.create("app", "Panel", QUrl::fromLocalFile("./Panel.qml"));
    creator.setType("Panel");
    auto ptr = creator.take();

    QString errorString;
    dfmgui::AppletManager::instance()->registeApplet(ptr, &errorString);
    qWarning() << errorString;

    // child
    creator.create("plugin", "AppletItemEx", QUrl::fromLocalFile("./AppletItemEx.qml"));
    creator.setParent("app.Panel");
    ptr = creator.take();
    dfmgui::AppletManager::instance()->registeApplet(ptr, &errorString);
    qWarning() << errorString;

    creator.create("plugin", "AppletItemEx2", QUrl::fromLocalFile("./AppletItemEx.qml"));
    creator.setParent("app.Panel");
    ptr = creator.take();
    dfmgui::AppletManager::instance()->registeApplet(ptr, &errorString);
    qWarning() << errorString;

    auto handle = dfmgui::WindowManager::instance()->createWindow("app", "Panel");
    handle->window()->show();

    handle->window()->dumpObjectTree();

    auto handle2 = dfmgui::WindowManager::instance()->createWindow("app", "Panel", { { "flag", 2 } });
    handle2->window()->show();

    int ret = app.exec();
    return ret;
}
