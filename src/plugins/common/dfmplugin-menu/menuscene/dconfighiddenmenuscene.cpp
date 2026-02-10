// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfighiddenmenuscene.h"
#include "private/dconfighiddenmenuscene_p.h"
#include "utils/menuhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QMenu>
#include <QStringList>
#include <QApplication>
#include <QDebug>

DPMENU_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;

DConfigHiddenMenuScenePrivate::DConfigHiddenMenuScenePrivate(DConfigHiddenMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

DConfigHiddenMenuScene::DConfigHiddenMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

QString dfmplugin_menu::DConfigHiddenMenuScene::name() const
{
    return DConfigHiddenMenuCreator::name();
}

bool DConfigHiddenMenuScene::initialize(const QVariantHash &params)
{
    auto currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    if (currentDir.isValid()) {
        // extend menu scene
        if (Helper::isHiddenExtMenu(currentDir))
            disableScene();
    }

    return true;
}

void dfmplugin_menu::DConfigHiddenMenuScene::updateState(QMenu *parent)
{
    updateActionHidden(parent);
    AbstractMenuScene::updateState(parent);
}

void DConfigHiddenMenuScene::disableScene()
{
    fmDebug() << "disable extend menu scene..";
    static const QSet<QString> extendScenes { "OemMenu", "ExtendMenu" };
    // this scene must be the sibling of extendScenes.
    if (auto parent = dynamic_cast<AbstractMenuScene *>(this->parent())) {
        auto subs = parent->subscene();
        for (auto sub : subs) {
            if (extendScenes.contains(sub->name())) {
                parent->removeSubscene(sub);
                delete sub;
            }
        }
    }
}

void DConfigHiddenMenuScene::updateActionHidden(QMenu *parent)
{
    static const QMap<QString, QString> appKeyMap {
        { "dde-file-manager", "dfm.menu.action.hidden" },
        { "org.deepin.dde-shell", "dd.menu.action.hidden" },
        { "dde-select-dialog-x11", "dfd.menu.action.hidden" },
        { "dde-select-dialog-wayland", "dfd.menu.action.hidden" },
        { "dde-file-dialog", "dfd.menu.action.hidden" },
    };

    auto hiddenActions = DConfigManager::instance()->value(kDefaultCfgPath, appKeyMap.value(qApp->applicationName())).toStringList();
    if (hiddenActions.isEmpty())
        return;

    fmDebug() << "menu: hidden actions: " << hiddenActions;

    QList<QMenu *> menus { parent };
    do {
        auto menu = menus.takeFirst();
        auto actions = menu->actions();
        for (int i = actions.count() - 1; i >= 0; --i) {
            auto action = actions.at(i);
            const QString &id = action->property(ActionPropertyKey::kActionID).toString();
            if (!id.isEmpty() && hiddenActions.contains(id))
                action->setVisible(false);

            if (auto subMenu = action->menu())
                menus.append(subMenu);
        }
    } while (menus.count() > 0);
}

AbstractMenuScene *DConfigHiddenMenuCreator::create()
{
    return new DConfigHiddenMenuScene();
}
