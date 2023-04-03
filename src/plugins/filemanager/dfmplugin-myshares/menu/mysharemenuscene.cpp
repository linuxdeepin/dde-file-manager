// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mysharemenuscene.h"
#include "private/mysharemenuscene_p.h"
#include "events/shareeventscaller.h"

#include <dfm-base/dfm_menu_defines.h>
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <QMenu>
#include <QDebug>

using namespace dfmplugin_myshares;

MyShareMenuScene::MyShareMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new MyShareMenuScenePrivate(this))
{
}

MyShareMenuScene::~MyShareMenuScene()
{
}

QString MyShareMenuScene::name() const
{
    return MyShareMenuCreator::name();
}

bool MyShareMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    QList<AbstractMenuScene *> currentScene;
    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene("ActionIconManager"))
        currentScene << actionIconManagerScene;

    currentScene << subScene;
    setSubscene(currentScene);
    AbstractMenuScene::initialize(params);

    return true;
}

bool MyShareMenuScene::create(QMenu *parent)
{
    d->createFileMenu(parent);
    AbstractMenuScene::create(parent);
    QList<QAction *> list = parent->actions();
    QAction *actPinToQuickAccess = nullptr;
    QAction *actProperty = nullptr;
    for (QAction *act : list) {
        if (act->property(ActionPropertyKey::kActionID).toString() == "add-bookmark")
            actPinToQuickAccess = act;
        if (act->property(ActionPropertyKey::kActionID).toString() == MySharesActionId::kShareProperty)
            actProperty = act;
    }
    if (actPinToQuickAccess && actProperty) {
        list.removeOne(actPinToQuickAccess);
        parent->insertAction(actProperty, actPinToQuickAccess);
    }

    return true;
}

void MyShareMenuScene::updateState(QMenu *parent)
{
    return AbstractMenuScene::updateState(parent);
}

bool MyShareMenuScene::triggered(QAction *action)
{
    do {
        if (!action)
            return false;

        QString id = action->property(ActionPropertyKey::kActionID).toString();
        if (!d->predicateAction.contains(id))
            break;

        return d->triggered(id);
    } while (false);
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *MyShareMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<MyShareMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AbstractMenuScene *MyShareMenuCreator::create()
{
    return new MyShareMenuScene();
}

MyShareMenuScenePrivate::MyShareMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[MySharesActionId::kOpenShareFolder] = tr("Open");
    predicateName[MySharesActionId::kOpenShareInNewWin] = tr("Open in new window");
    predicateName[MySharesActionId::kOpenShareInNewTab] = tr("Open in new tab");
    predicateName[MySharesActionId::kCancleSharing] = tr("Cancel sharing");
    predicateName[MySharesActionId::kShareProperty] = tr("Properties");
}

void MyShareMenuScenePrivate::createFileMenu(QMenu *parent)
{
    if (isEmptyArea)
        return;

    if (!parent)
        return;

    auto act = parent->addAction(predicateName[MySharesActionId::kOpenShareFolder]);
    act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareFolder);
    predicateAction[MySharesActionId::kOpenShareFolder] = act;

    if (selectFiles.count() == 1) {
        act = parent->addAction(predicateName[MySharesActionId::kOpenShareInNewWin]);
        act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareInNewWin);
        predicateAction[MySharesActionId::kOpenShareInNewWin] = act;

        act = parent->addAction(predicateName[MySharesActionId::kOpenShareInNewTab]);
        act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareInNewTab);
        predicateAction[MySharesActionId::kOpenShareInNewTab] = act;
        parent->addSeparator();

        act = parent->addAction(predicateName[MySharesActionId::kCancleSharing]);
        act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kCancleSharing);
        predicateAction[MySharesActionId::kCancleSharing] = act;
    }
    parent->addSeparator();

    act = parent->addAction(predicateName[MySharesActionId::kShareProperty]);
    act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kShareProperty);
    predicateAction[MySharesActionId::kShareProperty] = act;
}

bool MyShareMenuScenePrivate::triggered(const QString &id)
{
    if (!predicateAction.contains(id))
        return false;

    if (id == MySharesActionId::kOpenShareFolder) {
        auto mode = selectFiles.count() > 1 ? ShareEventsCaller::kOpenInNewWindow : ShareEventsCaller::kOpenInCurrentWindow;
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, mode);
    } else if (id == MySharesActionId::kOpenShareInNewWin) {
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, ShareEventsCaller::kOpenInNewWindow);
    } else if (id == MySharesActionId::kOpenShareInNewTab) {
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, ShareEventsCaller::kOpenInNewTab);
    } else if (id == MySharesActionId::kCancleSharing) {
        if (selectFiles.count() == 0)
            return false;
        ShareEventsCaller::sendCancelSharing(selectFiles.first());
    } else if (id == MySharesActionId::kShareProperty) {
        ShareEventsCaller::sendShowProperty(selectFiles);
    } else {
        return false;
    }
    return true;
}
