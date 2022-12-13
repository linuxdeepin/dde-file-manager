/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "private/oemmenuscene_p.h"
#include "oemmenu.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-framework/dpf.h"

#include <QMenu>
#include <QDebug>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *OemMenuCreator::create()
{
    return new OemMenuScene();
}

OemMenuScenePrivate::OemMenuScenePrivate(OemMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

QList<QAction *> OemMenuScenePrivate::childActions(QAction *action)
{
    QList<QAction *> actions;

    if (action->menu()) {
        auto tempChildActions = action->menu()->actions();
        for (auto childAction : tempChildActions) {
            actions << childAction;
            actions << childActions(childAction);
        }
    }

    return actions;
}

OemMenuScene::OemMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new OemMenuScenePrivate(this))
{
}

QString OemMenuScene::name() const
{
    return OemMenuCreator::name();
}

bool OemMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *OemMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->oemActions.contains(action) || d->oemChildActions.contains(action))
        return const_cast<OemMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OemMenuScene::create(QMenu *parent)
{
    d->oemActions.clear();
    d->oemChildActions.clear();

    if (d->isEmptyArea)
        d->oemActions = OemMenu::instance()->emptyActions(d->currentDir, d->onDesktop);
    else
        d->oemActions = OemMenu::instance()->normalActions(d->selectFiles, d->onDesktop);

    for (auto action : d->oemActions) {
        parent->addAction(action);
        d->oemChildActions.append(d->childActions(action));
    }

    return AbstractMenuScene::create(parent);
}

void OemMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    AbstractMenuScene::updateState(parent);
}

bool OemMenuScene::triggered(QAction *action)
{
    if (!d->oemActions.contains(action) && !d->oemChildActions.contains(action))
        return AbstractMenuScene::triggered(action);

    QPair<QString, QStringList> runable = OemMenu::instance()->makeCommand(action, d->currentDir, d->focusFile, d->selectFiles);
    if (!runable.first.isEmpty())
        return UniversalUtils::runCommand(runable.first, runable.second);

    return AbstractMenuScene::triggered(action);
}
