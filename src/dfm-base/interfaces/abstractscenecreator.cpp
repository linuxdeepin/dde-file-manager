// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractscenecreator.h>

using namespace dfmbase;

AbstractSceneCreator::AbstractSceneCreator()
{
}

AbstractSceneCreator::~AbstractSceneCreator()
{
}

bool AbstractSceneCreator::addChild(const QString &scene)
{
    if (scene.isEmpty())
        return false;

    if (!children.contains(scene))
        children.append(scene);
    return true;
}

void AbstractSceneCreator::removeChild(const QString &scene)
{
    children.removeOne(scene);
}
