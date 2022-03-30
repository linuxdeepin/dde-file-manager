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
#include "private/openfilemenuscene_p.h"
#include "action_defines.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/base/schemefactory.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *OpenFileMenuCreator::create()
{
    return new OpenFileMenuScene();
}

OpenFileMenuScenePrivate::OpenFileMenuScenePrivate(OpenFileMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kRename] = tr("Rename");
}

OpenFileMenuScene::OpenFileMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new OpenFileMenuScenePrivate(this))
{
}

QString OpenFileMenuScene::name() const
{
    return OpenFileMenuCreator::name();
}

bool OpenFileMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toString();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toString();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).toStringList();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    // 文件不存在，则无文件相关菜单项
    if (d->selectFiles.isEmpty())
        return false;

    QString errString;
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(QUrl(d->focusFile), true, &errString);
    if (d->focusFileInfo.isNull()) {
        qDebug() << errString;
        return false;
    }

    return true;
}

AbstractMenuScene *OpenFileMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<OpenFileMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OpenFileMenuScene::create(QMenu *parent)
{
    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kRename));
    d->predicateAction[ActionID::kRename] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRename));
    return true;
}

void OpenFileMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;
}

bool OpenFileMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)

    // TODO(Lee or others):
    return false;
}
