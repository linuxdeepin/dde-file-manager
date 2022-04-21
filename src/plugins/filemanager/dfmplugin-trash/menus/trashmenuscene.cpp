/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#include "trashmenuscene.h"
#include "private/trashmenuscene_p.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"

#include "services/common/menu/menu_defines.h"
#include "dfm-base/base/device/devicecontroller.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QMenu>

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *TrashMenuCreator::create()
{
    return new TrashMenuScene();
}

TrashMenuScene::TrashMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new TrashMenuScenePrivate(this))
{
}

TrashMenuScene::~TrashMenuScene()
{
}

QString TrashMenuScene::name() const
{
    return TrashMenuCreator::name();
}

bool TrashMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE

    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();

    if (!d->isEmptyArea) {
        if (d->selectFiles.isEmpty() || !d->focusFile.isValid() || !d->currentDir.isValid()) {
            qDebug() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
            return false;
        }

        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
    }

    return true;
}

bool TrashMenuScene::create(QMenu *parent)
{
    DSC_USE_NAMESPACE

    if (d->isEmptyArea) {
        auto isDisabled = TrashHelper::isEmpty() || !UniversalUtils::urlEquals(d->currentDir, TrashHelper::rootUrl());

        auto act = parent->addAction(d->predicateName[TrashActionId::kRestoreAll]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kRestoreAll);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kRestoreAll] = act;

        act = parent->addAction(d->predicateName[TrashActionId::kEmptyTrash]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kEmptyTrash);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kEmptyTrash] = act;
        parent->addSeparator();
    } else {
        auto act = parent->addAction(d->predicateName[TrashActionId::kRestore]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kRestore);
        d->predicateAction[TrashActionId::kRestore] = act;
    }
    return true;
}

void TrashMenuScene::updateState(QMenu *parent)
{
}

bool TrashMenuScene::triggered(QAction *action)
{
    DSC_USE_NAMESPACE
    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actId))
        return false;

    if (actId == TrashActionId::kRestore)
        TrashFileHelper::restoreFromTrashHandle(0, d->selectFiles, AbstractJobHandler::JobFlag::kRevocation);
    else if (actId == TrashActionId::kRestoreAll)
        TrashFileHelper::restoreFromTrashHandle(0, { d->currentDir }, AbstractJobHandler::JobFlag::kRevocation);
    else if (actId == TrashActionId::kEmptyTrash)
        TrashHelper::emptyTrash();

    return true;
}

AbstractMenuScene *TrashMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<TrashMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

TrashMenuScenePrivate::TrashMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[TrashActionId::kRestore] = tr("Restore");
    predicateName[TrashActionId::kRestoreAll] = tr("Restore all");
    predicateName[TrashActionId::kEmptyTrash] = tr("Empty trash");
}
