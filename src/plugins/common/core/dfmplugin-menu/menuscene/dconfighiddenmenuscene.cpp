// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfighiddenmenuscene.h"
#include "private/dconfighiddenmenuscene_p.h"
#include "utils/menuhelper.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <QMenu>
#include <QStringList>
#include <QApplication>
#include <QDebug>

DPMENU_USE_NAMESPACE

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
        if (Helper::isHiddenExtMenuByDConfig(currentDir))
            diableScene();
    }

    return true;
}

void dfmplugin_menu::DConfigHiddenMenuScene::updateState(QMenu *parent)
{
    updateMenuHidden(parent);
    updateActionHidden(parent);
    AbstractMenuScene::updateState(parent);
}

void DConfigHiddenMenuScene::diableScene()
{
    qDebug() << "disable extend menu scene..";
    static const QSet<QString> extendScenes{"OemMenu", "ExtendMenu"};
    // this scene must be the child of root scene.
    if (auto parent = dynamic_cast<AbstractMenuScene *>(this->parent())) {
        auto subs = parent->subscene();
        for (auto sub : subs) {
            if (extendScenes.contains(sub->name())) {
                parent->removeSubscene(sub);
                qInfo() << "delete scene" << sub->name();
                delete sub;
            }
        }
    }
}

void DConfigHiddenMenuScene::updateActionHidden(QMenu *parent)
{
    static const QMap<QString, QString> appKeyMap {
        { "dde-file-manager", "dfm.menu.action.hidden" },
        { "dde-desktop", "dd.menu.action.hidden" },
        { "dde-select-dialog-x11", "dfd.menu.action.hidden" },
        { "dde-select-dialog-wayland", "dfd.menu.action.hidden" },
        { "dde-file-dialog", "dfd.menu.action.hidden" },
    };

    auto hiddenActions = DConfigManager::instance()->value(kDefaultCfgPath, appKeyMap.value(qApp->applicationName())).toStringList();
    if (hiddenActions.isEmpty())
        return;

    qDebug() << "menu: hidden actions: " << hiddenActions;

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

void DConfigHiddenMenuScene::updateMenuHidden(QMenu *parent)
{
    auto hiddenMenus = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hidden").toStringList();
    if (hiddenMenus.isEmpty())
        return;

    qDebug() << "menu: hidden menu in app: " << qApp->applicationName() << hiddenMenus;
    if ((hiddenMenus.contains("dde-file-manager") && qApp->applicationName() == "dde-file-manager")
        || (hiddenMenus.contains("dde-desktop") && qApp->applicationName() == "dde-desktop")
        || (hiddenMenus.contains("dde-file-dialog") && qApp->applicationName().startsWith("dde-select-dialog"))) {
        auto acts = parent->actions();
        for (auto act : acts)
            act->setVisible(false);
    }
}

AbstractMenuScene *DConfigHiddenMenuCreator::create()
{
    return new DConfigHiddenMenuScene();
}
