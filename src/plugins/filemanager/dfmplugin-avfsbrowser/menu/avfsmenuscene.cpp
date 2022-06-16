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
#include "avfsmenuscene.h"
#include "private/avfsmenuscene_p.h"

DPAVFSBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AvfsMenuScene::AvfsMenuScene(QObject *parent)
    : AbstractMenuScene((parent)), d(new AvfsMenuScenePrivate(this))
{
}

AbstractMenuScene *AvfsMenuSceneCreator::create()
{
    return nullptr;
}

QString AvfsMenuScene::name() const
{
    return AvfsMenuSceneCreator::name();
}

bool AvfsMenuScene::initialize(const QVariantHash &params)
{
    return false;
}

bool AvfsMenuScene::create(QMenu *parent)
{
    return false;
}

void AvfsMenuScene::updateState(QMenu *parent)
{
}

bool AvfsMenuScene::triggered(QAction *action)
{
    return false;
}

AbstractMenuScene *AvfsMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<AvfsMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AvfsMenuScenePrivate::AvfsMenuScenePrivate(AvfsMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}
