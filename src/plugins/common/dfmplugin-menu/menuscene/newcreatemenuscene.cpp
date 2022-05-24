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
#include "action_defines.h"

#include "services/common/menu/menu_defines.h"

#include "dfm-base/base/schemefactory.h"

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *NewCreateMenuCreator::create()
{
    return new NewCreateMenuScene();
}

NewCreateMenuScenePrivate::NewCreateMenuScenePrivate(NewCreateMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kNewFolder] = tr("New folder");
    predicateName[ActionID::kNewDoc] = tr("New document");
    predicateName[ActionID::kNewOfficeText] = tr("Office Text");
    predicateName[ActionID::kNewSpreadsheets] = tr("Spreadsheets");
    predicateName[ActionID::kNewPresentation] = tr("Presentation");
    predicateName[ActionID::kNewPlainText] = tr("Plain Text");
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
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    if (!d->currentDir.isValid())
        return false;

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *NewCreateMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<NewCreateMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool NewCreateMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kNewFolder));
    d->predicateAction[ActionID::kNewFolder] = tempAction;
    // set action id
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewFolder));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kNewDoc));
    d->predicateAction[ActionID::kNewDoc] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewDoc));

    QMenu *newDocSubMenu = new QMenu(parent);
    tempAction->setMenu(newDocSubMenu);

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewOfficeText));
    d->predicateAction[ActionID::kNewOfficeText] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewOfficeText));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewSpreadsheets));
    d->predicateAction[ActionID::kNewSpreadsheets] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewSpreadsheets));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewPresentation));
    d->predicateAction[ActionID::kNewPresentation] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewPresentation));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewPlainText));
    d->predicateAction[ActionID::kNewPlainText] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewPlainText));

    return AbstractMenuScene::create(parent);
}

void NewCreateMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    auto curDirInfo = InfoFactory::create<AbstractFileInfo>(d->currentDir);
    if (!curDirInfo)
        return;
    curDirInfo->refresh();
    if (!curDirInfo->isWritable()) {
        auto actions = parent->actions();
        for (auto act : actions) {
            const auto &actId = act->property(ActionPropertyKey::kActionID);
            if (ActionID::kNewFolder == actId) {
                act->setDisabled(true);
            } else if (ActionID::kNewDoc == actId) {
                act->setDisabled(true);
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool NewCreateMenuScene::triggered(QAction *action)
{
    // TODO(Lee or others):

    QString id = d->predicateAction.key(action);
    if (d->predicateName.contains(id)) {
        if (id == ActionID::kNewFolder) {

        } else if (id == ActionID::kNewDoc) {

        } else if (id == ActionID::kNewOfficeText) {

        } else if (id == ActionID::kNewSpreadsheets) {

        } else if (id == ActionID::kNewPresentation) {

        } else if (id == ActionID::kNewPlainText) {
        }
        return true;
    }

    return AbstractMenuScene::triggered(action);
}
