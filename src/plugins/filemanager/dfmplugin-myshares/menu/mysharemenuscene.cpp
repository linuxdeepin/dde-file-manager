// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mysharemenuscene.h"
#include "private/mysharemenuscene_p.h"
#include "events/shareeventscaller.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <QMenu>
#include <QDebug>

using namespace dfmplugin_myshares;

MyShareMenuScene::MyShareMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new MyShareMenuScenePrivate(this))
{
    fmDebug() << "MyShareMenuScene constructed";
}

MyShareMenuScene::~MyShareMenuScene()
{
    fmDebug() << "MyShareMenuScene destructed";
}

QString MyShareMenuScene::name() const
{
    return MyShareMenuCreator::name();
}

bool MyShareMenuScene::initialize(const QVariantHash &params)
{
    fmDebug() << "Initializing MyShareMenuScene with parameters";

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

    fmDebug() << "MyShareMenuScene initialization completed";
    return true;
}

bool MyShareMenuScene::create(QMenu *parent)
{
    fmDebug() << "Creating MyShareMenuScene menu";

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

    fmDebug() << "MyShareMenuScene menu creation completed";
    return true;
}

void MyShareMenuScene::updateState(QMenu *parent)
{
    return AbstractMenuScene::updateState(parent);
}

bool MyShareMenuScene::triggered(QAction *action)
{
    do {
        if (!action) {
            fmWarning() << "Null action triggered in MyShareMenuScene";
            return false;
        }

        QString id = action->property(ActionPropertyKey::kActionID).toString();
        if (!d->predicateAction.contains(id))
            break;

        return d->triggered(id);
    } while (false);
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *MyShareMenuScene::scene(QAction *action) const
{
    if (action == nullptr) {
        fmDebug() << "Scene requested for null action";
        return nullptr;
    }

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
    predicateName[MySharesActionId::kOpenShareFolder] = tr("&Open");
    predicateName[MySharesActionId::kOpenShareInNewWin] = tr("Open in new window");
    predicateName[MySharesActionId::kOpenShareInNewTab] = tr("Open in new tab");
    predicateName[MySharesActionId::kCancleSharing] = tr("Cancel sharing");
    predicateName[MySharesActionId::kShareProperty] = tr("P&roperties");
}

void MyShareMenuScenePrivate::createFileMenu(QMenu *parent)
{
    fmDebug() << "Creating file menu for MyShares";

    if (isEmptyArea) {
        fmDebug() << "Empty area detected, skipping file menu creation";
        return;
    }

    if (!parent) {
        fmWarning() << "Null parent menu provided, cannot create file menu";
        return;
    }

    auto act = parent->addAction(predicateName[MySharesActionId::kOpenShareFolder]);
    act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareFolder);
    predicateAction[MySharesActionId::kOpenShareFolder] = act;

    if (selectFiles.count() == 1) {
        auto info = InfoFactory::create<FileInfo>(selectFiles.first());
        if (info && info->isAttributes(OptInfoType::kIsDir)) {
            act = parent->addAction(predicateName[MySharesActionId::kOpenShareInNewWin]);
            act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareInNewWin);
            predicateAction[MySharesActionId::kOpenShareInNewWin] = act;

            act = parent->addAction(predicateName[MySharesActionId::kOpenShareInNewTab]);
            act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kOpenShareInNewTab);
            predicateAction[MySharesActionId::kOpenShareInNewTab] = act;
            parent->addSeparator();

            bool shared = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_IsPathShared", info->pathOf(PathInfoType::kAbsoluteFilePath)).toBool();
            if (shared) {
                act = parent->addAction(predicateName[MySharesActionId::kCancleSharing]);
                act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kCancleSharing);
                predicateAction[MySharesActionId::kCancleSharing] = act;
            }
        }
    }
    parent->addSeparator();

    act = parent->addAction(predicateName[MySharesActionId::kShareProperty]);
    act->setProperty(ActionPropertyKey::kActionID, MySharesActionId::kShareProperty);
    predicateAction[MySharesActionId::kShareProperty] = act;
}

bool MyShareMenuScenePrivate::triggered(const QString &id)
{
    fmDebug() << "Processing triggered action:" << id;

    if (!predicateAction.contains(id)) {
        fmWarning() << "Action ID not found in predicate actions:" << id;
        return false;
    }

    if (id == MySharesActionId::kOpenShareFolder) {
        auto mode = selectFiles.count() > 1 ? ShareEventsCaller::kOpenInNewWindow : ShareEventsCaller::kOpenInCurrentWindow;
        fmInfo() << "Opening share folder(s) for" << selectFiles.count() << "files, mode:" << (mode == ShareEventsCaller::kOpenInNewWindow ? "new window" : "current window");
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, mode);
    } else if (id == MySharesActionId::kOpenShareInNewWin) {
        fmInfo() << "Opening share in new window for" << selectFiles.count() << "files";
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, ShareEventsCaller::kOpenInNewWindow);
    } else if (id == MySharesActionId::kOpenShareInNewTab) {
        fmInfo() << "Opening share in new tab for" << selectFiles.count() << "files";
        ShareEventsCaller::sendOpenDirs(windowId, selectFiles, ShareEventsCaller::kOpenInNewTab);
    } else if (id == MySharesActionId::kCancleSharing) {
        if (selectFiles.count() == 0) {
            fmWarning() << "No files selected for cancel sharing operation";
            return false;
        }
        fmInfo() << "Canceling sharing for file:" << selectFiles.first();
        ShareEventsCaller::sendCancelSharing(selectFiles.first());
    } else if (id == MySharesActionId::kShareProperty) {
        fmInfo() << "Showing properties for" << selectFiles.count() << "selected files";
        ShareEventsCaller::sendShowProperty(selectFiles);
    } else {
        fmWarning() << "Unknown action ID triggered:" << id;
        return false;
    }
    return true;
}
