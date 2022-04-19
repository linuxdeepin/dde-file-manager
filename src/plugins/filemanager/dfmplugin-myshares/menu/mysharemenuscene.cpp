/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "mysharemenuscene.h"
#include "private/mysharemenuscene_p.h"
#include "events/shareeventscaller.h"

#include "services/common/menu/menu_defines.h"

#include <QMenu>
#include <QDebug>

DPMYSHARES_USE_NAMESPACE

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
    DSC_USE_NAMESPACE
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    AbstractMenuScene::initialize(params);

    return true;
}

bool MyShareMenuScene::create(QMenu *parent)
{
    d->createFileMenu(parent);
    AbstractMenuScene::create(parent);
    return true;
}

void MyShareMenuScene::updateState(QMenu *parent)
{
    return AbstractMenuScene::updateState(parent);
}

bool MyShareMenuScene::triggered(QAction *action)
{
    do {
        DSC_USE_NAMESPACE
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

    DSC_USE_NAMESPACE
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
