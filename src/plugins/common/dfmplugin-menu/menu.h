/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef MENU_H
#define MENU_H

#include "dfmplugin_menu_global.h"

#include "dfm-base/interfaces/abstractscenecreator.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_menu {

class MenuHandle : public QObject
{
    Q_OBJECT
public:
    explicit MenuHandle(QObject *parent = nullptr);
    ~MenuHandle();
    bool init();
public slots:
    // scene
    bool contains(const QString &name);
    bool registerScene(const QString &name, DFMBASE_NAMESPACE::AbstractSceneCreator *creator);
    DFMBASE_NAMESPACE::AbstractSceneCreator *unregisterScene(const QString &name);
    bool bind(const QString &name, const QString &parent);
    void unbind(const QString &name, const QString &parent = QString());
    DFMBASE_NAMESPACE::AbstractMenuScene *createScene(const QString &name);

    // utils
    QVariantHash perfectMenuParams(const QVariantHash &params);
private slots:
    void publishSceneAdded(const QString &scene);
    void publishSceneRemoved(const QString &scene);
protected:
    void createSubscene(DFMBASE_NAMESPACE::AbstractSceneCreator *creator, DFMBASE_NAMESPACE::AbstractMenuScene *parent);
private:
    QHash<QString, DFMBASE_NAMESPACE::AbstractSceneCreator *> creators;
    QReadWriteLock locker;
};

class Menu : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "menu.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private:
    MenuHandle *handle = nullptr;
private:
    DPF_EVENT_NAMESPACE(DPMENU_NAMESPACE)
    // MenuScene
    DPF_EVENT_REG_SIGNAL(signal_MenuScene_SceneAdded)
    DPF_EVENT_REG_SIGNAL(signal_MenuScene_SceneRemoved)

    DPF_EVENT_REG_SLOT(slot_MenuScene_Contains)
    DPF_EVENT_REG_SLOT(slot_MenuScene_RegisterScene)
    DPF_EVENT_REG_SLOT(slot_MenuScene_UnregisterScene)
    DPF_EVENT_REG_SLOT(slot_MenuScene_Bind)
    DPF_EVENT_REG_SLOT(slot_MenuScene_Unbind)
    DPF_EVENT_REG_SLOT(slot_MenuScene_CreateScene)

    // menu utils
    DPF_EVENT_REG_SLOT(slot_Menu_PerfectParams)
};

}

#endif   // MENU_H
