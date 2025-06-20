// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

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

bool AbstractMenuScene::actionFilter(AbstractMenuScene *caller, QAction *action)
{
    // just filter the parent of this scene.
    //! must not called children's 'actionFilter' function in this function.
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

void AbstractMenuScene::setSubscene(const QList<AbstractMenuScene *> &scenes)
{
    //! if these already were subscenes before setting, the caller is responsible for managing their lifecycle

    subScene = scenes;
    for (auto scene : scenes)
        scene->setParent(this);
}
