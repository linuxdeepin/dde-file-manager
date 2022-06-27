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
#include "sharemenuscene.h"
#include "private/sharemenuscene_p.h"
#include "private/shareutils.h"

#include "services/common/usershare/usershareservice.h"
#include "services/common/propertydialog/property_defines.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_menu_defines.h"

#include <dfm-framework/event/event.h>

#include <QFileInfo>
#include <QMenu>

using namespace dfmplugin_dirshare;

ShareMenuScenePrivate::ShareMenuScenePrivate(dfmbase::AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void ShareMenuScenePrivate::addShare(const QUrl &url)
{
    QList<QUrl> urls { url };
    dpfSignalDispatcher->publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog, urls);
}

ShareMenuScene::ShareMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ShareMenuScenePrivate(this))
{
}

ShareMenuScene::~ShareMenuScene()
{
}

QString ShareMenuScene::name() const
{
    return ShareMenuCreator::name();
}

bool ShareMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    d->predicateName.insert(ShareActionId::kActAddShareKey, tr("Share folder"));
    d->predicateName.insert(ShareActionId::kActRemoveShareKey, tr("Cancel sharing"));

    if (d->selectFiles.count() != 1)
        return false;

    QUrl u(d->selectFiles.first());
    if (u.scheme() != Global::Scheme::kFile)
        return false;

    auto info = InfoFactory::create<AbstractFileInfo>(u, true);
    if (!info->isDir())
        return false;

    return AbstractMenuScene::initialize(params);
}

bool ShareMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (d->selectFiles.count() != 1)
        return false;

    auto info = InfoFactory::create<AbstractFileInfo>(d->selectFiles.first(), true);
    if (info->isDir()) {
        DSC_USE_NAMESPACE
        if (UserShareService::service()->isSharedPath(info->absoluteFilePath())) {
            auto act = parent->addAction(d->predicateName[ShareActionId::kActRemoveShareKey]);
            act->setProperty(ActionPropertyKey::kActionID, ShareActionId::kActRemoveShareKey);
            d->predicateAction.insert(ShareActionId::kActRemoveShareKey, act);
        } else {
            if (ShareUtils::canShare(info)) {
                auto act = parent->addAction(d->predicateName[ShareActionId::kActAddShareKey]);
                act->setProperty(ActionPropertyKey::kActionID, ShareActionId::kActAddShareKey);
                d->predicateAction.insert(ShareActionId::kActAddShareKey, act);
            }
        }
    }

    return AbstractMenuScene::create(parent);
}

void ShareMenuScene::updateState(QMenu *parent)
{
    DSC_USE_NAMESPACE
    auto actLst = parent->actions();
    QAction *shareOrUnshare = nullptr;
    QAction *symlinkAct = nullptr;

    for (const auto act : actLst) {
        if (act->property(ActionPropertyKey::kActionID).toString() == "create-system-link")
            symlinkAct = act;
        if (d->predicateAction.values().contains(act))
            shareOrUnshare = act;
    }

    if (shareOrUnshare && symlinkAct) {
        parent->removeAction(shareOrUnshare);
        parent->insertSeparator(symlinkAct);
        parent->insertAction(symlinkAct, shareOrUnshare);
        parent->insertSeparator(symlinkAct);
    }

    AbstractMenuScene::updateState(parent);
}

bool ShareMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    if (d->selectFiles.count() != 1)
        return false;

    QUrl u = d->selectFiles.first();
    if (u.scheme() != Global::Scheme::kFile)
        return false;

    if (!QFileInfo(u.path()).isDir())
        return false;

    DSC_USE_NAMESPACE
    QString key = action->property(ActionPropertyKey::kActionID).toString();
    if (key == ShareActionId::kActAddShareKey) {
        d->addShare(u);
        return true;
    } else if (key == ShareActionId::kActRemoveShareKey) {
        UserShareService::service()->removeShare(u.path());
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *ShareMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<ShareMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AbstractMenuScene *ShareMenuCreator::create()
{
    return new ShareMenuScene();
}
