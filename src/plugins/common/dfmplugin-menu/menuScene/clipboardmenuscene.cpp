/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
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
#include "private/clipboardmenuscene_p.h"

#include <dfm-base/utils/clipboard.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *ClipBoardMenuCreator::create()
{
    return new ClipBoardMenuScene();
}

ClipBoardMenuScenePrivate::ClipBoardMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[paste] = tr("Paste");
    predicateName[cut] = tr("Cut");
    predicateName[copy] = tr("Copy");
}

ClipBoardMenuScene::ClipBoardMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new ClipBoardMenuScenePrivate(this))
{
}

QString ClipBoardMenuScene::name() const
{
    return ClipBoardMenuCreator::name();
}

bool ClipBoardMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(kCurrentDir).toString();
    d->selectFiles = params.value(kSelectFiles).toStringList();
    d->focusFile = params.value(kFocusFile).toString();
    d->isEmptyArea = params.value(kIsEmptyArea).toBool();

    // 文件不存在，则无文件相关菜单项
    if (d->selectFiles.isEmpty() && d->focusFile.isEmpty() && d->currentDir.isEmpty())
        return false;

    return true;
}

AbstractMenuScene *ClipBoardMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->providSelfActionList.contains(action))
        return const_cast<ClipBoardMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ClipBoardMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.key(d->paste));
        d->providSelfActionList.append(tempAction);
        d->predicateAction[d->paste] = tempAction;
    } else {
        QAction *tempAction = parent->addAction(d->predicateName.key(d->cut));
        d->providSelfActionList.append(tempAction);
        d->predicateAction[d->cut] = tempAction;

        tempAction = parent->addAction(d->predicateName.key(d->copy));
        d->providSelfActionList.append(tempAction);
        d->predicateAction[d->copy] = tempAction;
    }

    return true;
}

void ClipBoardMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    bool clipBoardUnknow = ClipBoard::instance()->clipboardAction() == ClipBoard::kUnknownAction;
    d->predicateAction[d->paste]->setEnabled(clipBoardUnknow ? false : true);
}

bool ClipBoardMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)

    // TODO(Lee or others):
    return false;
}
