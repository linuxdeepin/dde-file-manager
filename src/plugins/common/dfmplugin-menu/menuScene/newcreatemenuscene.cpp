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
#include "newcreatemenuscene.h"

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
#include "private/newcreatemenuscene_p.h"

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *NewCreateMenuCreator::create()
{
    return new NewCreateMenuScene();
}

NewCreateMenuScenePrivate::NewCreateMenuScenePrivate(NewCreateMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[newFolder] = tr("New folder");
    predicateName[newDoc] = tr("New document");
    predicateName[officeText] = tr("Office Text");
    predicateName[spreadsheets] = tr("Spreadsheets");
    predicateName[presentation] = tr("Presentation");
    predicateName[plainText] = tr("Plain Text");
}

NewCreateMenuScene::NewCreateMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new NewCreateMenuScenePrivate(this))
{
}

QString NewCreateMenuScene::name() const
{
    return NewCreateMenuCreator::name();
}

bool NewCreateMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(kCurrentDir).toString();
    d->onDesktop = params.value(kOnDesktop).toBool();

    if (d->currentDir.isEmpty())
        return false;

    return true;
}

AbstractMenuScene *NewCreateMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->providSelfActionList.contains(action))
        return const_cast<NewCreateMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool NewCreateMenuScene::create(QMenu *parent)
{

    if (!parent)
        return false;

    QAction *tempAction = parent->addAction(d->predicateName.value(d->newFolder));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->newFolder] = tempAction;

    tempAction = parent->addAction(d->predicateName.value(d->newDoc));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->newFolder] = tempAction;

    QMenu *newDocSubMenu = new QMenu(parent);
    tempAction->setMenu(newDocSubMenu);

    tempAction = newDocSubMenu->addAction(d->predicateName.value(d->officeText));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->officeText] = tempAction;

    tempAction = newDocSubMenu->addAction(d->predicateName.value(d->spreadsheets));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->spreadsheets] = tempAction;

    tempAction = newDocSubMenu->addAction(d->predicateName.value(d->presentation));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->presentation] = tempAction;

    tempAction = newDocSubMenu->addAction(d->predicateName.value(d->plainText));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[d->plainText] = tempAction;

    return true;
}

void NewCreateMenuScene::updateState(QMenu *parent)
{
    Q_UNUSED(parent)
}

bool NewCreateMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO(Lee or others):
    return false;
}
