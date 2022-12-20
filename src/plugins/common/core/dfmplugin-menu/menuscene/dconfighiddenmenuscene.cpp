// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfighiddenmenuscene.h"
#include "private/dconfighiddenmenuscene_p.h"

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

void dfmplugin_menu::DConfigHiddenMenuScene::updateState(QMenu *parent)
{
    updateMenuHidden(parent);
    updateActionHidden(parent);
    AbstractMenuScene::updateState(parent);
}

void DConfigHiddenMenuScene::updateActionHidden(QMenu *parent)
{
    static const QMap<QString, QString> appKeyMap {
        { "dde-file-manager", "dfm.menu.action.hidden" },
        { "dde-desktop", "dd.menu.action.hidden" },
        { "dde-select-dialog-x11", "dfd.menu.action.hidden" },
        { "dde-select-dialog-wayland", "dfd.menu.action.hidden" },
        { "dde-select-dialog", "dfd.menu.action.hidden" },
    };

    auto hiddenActions = DConfigManager::instance()->value(kDefaultCfgPath, appKeyMap.value(qApp->applicationName())).toStringList();
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
