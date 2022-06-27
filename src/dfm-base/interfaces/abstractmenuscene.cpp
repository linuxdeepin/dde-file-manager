/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
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
#include "private/abstractmenuscene_p.h"

using namespace dfmbase;

AbstractMenuScenePrivate::AbstractMenuScenePrivate(AbstractMenuScene *qq)
    : QObject(qq)
{
}

bool AbstractMenuScenePrivate::initializeParamsIsValid()
{
    if (!isEmptyArea) {
        if (selectFiles.isEmpty() || !focusFile.isValid() || !currentDir.isValid())
            return false;
    }

    return true;
}

AbstractMenuScene::AbstractMenuScene(QObject *parent)
    : QObject(parent)
{
}

AbstractMenuScene::~AbstractMenuScene()
{
}

bool AbstractMenuScene::initialize(const QVariantHash &params)
{
    auto current = subScene;
    for (AbstractMenuScene *scene : current) {
        if (!scene->initialize(params)) {
            subScene.removeOne(scene);
            delete scene;
        }
    }

    return true;
}

bool AbstractMenuScene::create(QMenu *parent)
{
    for (AbstractMenuScene *scene : subScene)
        scene->create(parent);
    return true;
}

void AbstractMenuScene::updateState(QMenu *parent)
{
    for (AbstractMenuScene *scene : subScene)
        scene->updateState(parent);
}

bool AbstractMenuScene::triggered(QAction *action)
{
    for (AbstractMenuScene *scene : subScene)
        if (scene->triggered(action))
            return true;

    return false;
}

AbstractMenuScene *AbstractMenuScene::scene(QAction *action) const
{
    for (AbstractMenuScene *scene : subScene)
        if (auto from = scene->scene(action))
            return from;

    return nullptr;
}

bool AbstractMenuScene::addSubscene(AbstractMenuScene *scene)
{
    if (!scene)
        return false;

    scene->setParent(this);
    subScene.append(scene);
    return true;
}

void AbstractMenuScene::removeSubscene(AbstractMenuScene *scene)
{
    if (scene && scene->parent() == this)
        scene->setParent(nullptr);

    subScene.removeOne(scene);
}

void AbstractMenuScene::setSubscene(const QList<dfmbase::AbstractMenuScene *> &scenes)
{
    //! if these already were subscenes before setting, the caller is responsible for managing their lifecycle

    subScene = scenes;
    for (auto scene : scenes)
        scene->setParent(this);
}
