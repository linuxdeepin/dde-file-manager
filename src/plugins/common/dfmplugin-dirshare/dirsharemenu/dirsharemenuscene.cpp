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
#include "dirsharemenuscene.h"
#include "private/dirsharemenuscene_p.h"
#include "utils/usersharehelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_menu_defines.h"
#include "widget/sharecontrolwidget.h"

#include <dfm-framework/event/event.h>

#include <QFileInfo>
#include <QMenu>

using namespace dfmplugin_dirshare;
using ViewIntiCallback = std::function<void(QWidget *w, const QVariantHash &opt)>;

DirShareMenuScenePrivate::DirShareMenuScenePrivate(dfmbase::AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void DirShareMenuScenePrivate::addShare(const QUrl &url)
{
    QList<QUrl> urls { url };
    QVariantHash option;
    option.insert("Option_Key_Name", "DirShare");
    option.insert("Option_Key_ExtendViewExpand", true);
    option.insert("Option_Key_BasicInfoExpand", false);

    ViewIntiCallback initFun = { ShareControlWidget::setOption };
    option.insert("Option_Key_ViewInitCalback", QVariant::fromValue(initFun));

    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, option);
}

DirShareMenuScene::DirShareMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new DirShareMenuScenePrivate(this))
{
}

DirShareMenuScene::~DirShareMenuScene()
{
}

QString DirShareMenuScene::name() const
{
    return DirShareMenuCreator::name();
}

bool DirShareMenuScene::initialize(const QVariantHash &params)
{
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
    if (!info->isAttributes(AbstractFileInfo::FileIsType::kIsDir))
        return false;

    return AbstractMenuScene::initialize(params);
}

bool DirShareMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (d->selectFiles.count() != 1)
        return false;

    auto info = InfoFactory::create<AbstractFileInfo>(d->selectFiles.first(), true);
    if (info->isAttributes(AbstractFileInfo::FileIsType::kIsDir)) {
        bool shared = UserShareHelperInstance->isShared(info->pathInfo(PathInfo::kAbsoluteFilePath));
        if (shared) {
            auto act = parent->addAction(d->predicateName[ShareActionId::kActRemoveShareKey]);
            act->setProperty(ActionPropertyKey::kActionID, ShareActionId::kActRemoveShareKey);
            d->predicateAction.insert(ShareActionId::kActRemoveShareKey, act);
        } else {
            if (UserShareHelper::canShare(info)) {
                auto act = parent->addAction(d->predicateName[ShareActionId::kActAddShareKey]);
                act->setProperty(ActionPropertyKey::kActionID, ShareActionId::kActAddShareKey);
                d->predicateAction.insert(ShareActionId::kActAddShareKey, act);
            }
        }
    }

    return AbstractMenuScene::create(parent);
}

void DirShareMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
}

bool DirShareMenuScene::triggered(QAction *action)
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

    QString key = action->property(ActionPropertyKey::kActionID).toString();
    if (key == ShareActionId::kActAddShareKey) {
        d->addShare(u);
        return true;
    } else if (key == ShareActionId::kActRemoveShareKey) {
        UserShareHelperInstance->removeShareByPath(u.path());
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *DirShareMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<DirShareMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AbstractMenuScene *DirShareMenuCreator::create()
{
    return new DirShareMenuScene();
}

Q_DECLARE_METATYPE(ViewIntiCallback);
