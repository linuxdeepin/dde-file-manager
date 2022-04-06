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
#include "action_defines.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/utils/clipboard.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *ClipBoardMenuCreator::create()
{
    return new ClipBoardMenuScene();
}

ClipBoardMenuScenePrivate::ClipBoardMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kPaste] = tr("Paste");
    predicateName[ActionID::kCut] = tr("Cut");
    predicateName[ActionID::kCopy] = tr("Copy");
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
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    // 文件不存在，则无文件相关菜单项
    if (d->selectFiles.isEmpty() && !d->focusFile.isValid() && !d->currentDir.isValid())
        return false;

    return true;
}

AbstractMenuScene *ClipBoardMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<ClipBoardMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ClipBoardMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kPaste));
        d->predicateAction[ActionID::kPaste] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kPaste));
    } else {
        QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kCut));
        d->predicateAction[ActionID::kCut] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kCut));

        tempAction = parent->addAction(d->predicateName.value(ActionID::kCopy));
        d->predicateAction[ActionID::kCopy] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kCopy));
    }

    return true;
}

void ClipBoardMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (auto paste = d->predicateAction.value(ActionID::kPaste)) {
        bool clipBoardUnknow = ClipBoard::instance()->clipboardAction() == ClipBoard::kUnknownAction;
        paste->setEnabled(clipBoardUnknow ? false : true);
    }
}

bool ClipBoardMenuScene::triggered(QAction *action)
{
    // TODO(Lee or others):
    QString id = d->predicateAction.key(action);
    if (d->predicateName.contains(id)) {
        if (id == ActionID::kPaste) {

        } else if (id == ActionID::kCut) {

        } else if (id == ActionID::kCopy) {
        }
        return true;
    }

    return false;
}
