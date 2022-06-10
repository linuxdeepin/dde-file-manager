/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "tagdirmenuscene.h"
#include "private/tagdirmenusceneprivate.h"
#include "utils/tagmanager.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/menu/menuservice.h"

DPTAG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *TagDirMenuCreator::create()
{
    return new TagDirMenuScene();
}

TagDirMenuScene::TagDirMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new TagDirMenuScenePrivate(this))
{
}

TagDirMenuScene::~TagDirMenuScene()
{
}

QString TagDirMenuScene::name() const
{
    return TagDirMenuCreator::name();
}

bool TagDirMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dpfSlotChannel->push("dfmplugin_menu", "slot_PerfectMenuParams", params).value<QVariantHash>();
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    QList<AbstractMenuScene *> currentScene;
    if (d->isEmptyArea) {
        if (auto newCreateScene = MenuService::service()->createScene("SortAndDisplayMenu"))
            currentScene.append(newCreateScene);
    } else {
        if (auto newCreateScene = MenuService::service()->createScene("WorkspaceMenu"))
            currentScene.append(newCreateScene);
    }
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

void TagDirMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    AbstractMenuScene::updateState(parent);

    QList<QAction *> actions = parent->actions();
    QList<QAction *>::iterator itAction = actions.begin();
    for (; itAction != actions.end(); ++itAction) {
        if ((*itAction)->isSeparator())
            continue;

        const QString sceneName = scene(*itAction)->name();
        if (sceneName == "ExtendMenu" || sceneName == "OemMenu") {
            parent->removeAction(*itAction);
        }
    }
}

AbstractMenuScene *TagDirMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<TagDirMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}
