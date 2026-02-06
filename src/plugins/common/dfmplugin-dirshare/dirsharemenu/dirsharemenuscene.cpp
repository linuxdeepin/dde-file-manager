// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dirsharemenuscene.h"
#include "private/dirsharemenuscene_p.h"
#include "utils/usersharehelper.h"
#include "utils/anonymouspermissionmanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>
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
    option.insert("Option_Key_DisableCustomDialog", true);

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
    d->focusFile = d->selectFiles.first();
    if (d->focusFile.scheme() != Global::Scheme::kFile)
        return false;

    d->focusFileInfo = InfoFactory::create<FileInfo>(d->focusFile);
    if (d->focusFileInfo && !d->focusFileInfo->isAttributes(OptInfoType::kIsDir))
        return false;

    return AbstractMenuScene::initialize(params);
}

bool DirShareMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (d->selectFiles.count() != 1)
        return false;

    auto info = d->focusFileInfo;
    if (info && info->isAttributes(OptInfoType::kIsDir)) {
        bool shared = UserShareHelperInstance->isShared(info->pathOf(PathInfoType::kAbsoluteFilePath));
        if (shared) {
            auto act = parent->addAction(d->predicateName[ShareActionId::kActRemoveShareKey]);
            act->setProperty(ActionPropertyKey::kActionID, ShareActionId::kActRemoveShareKey);
            d->predicateAction.insert(ShareActionId::kActRemoveShareKey, act);
        } else {
            if (UserShareHelper::canShare(info) && !UserShareHelper::needDisableShareWidget(info)) {
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

    QUrl u = d->selectFiles.first();
    QString key = action->property(ActionPropertyKey::kActionID).toString();
    if (key == ShareActionId::kActAddShareKey) {
        d->addShare(u);
        return true;
    } else if (key == ShareActionId::kActRemoveShareKey) {
        QString filePath = u.path();

        // Get share info before removing to check if it's anonymous share
        auto shareInfo = UserShareHelperInstance->shareInfoByPath(filePath);
        bool wasAnonymous = shareInfo.value(ShareInfoKeys::kAnonymous).toBool();

        // Remove share
        bool success = UserShareHelperInstance->removeShareByPath(filePath);

        // Restore permissions if it was anonymous share
        if (success && wasAnonymous) {
            AnonymousPermissionManager::instance()->restoreDirectoryPermissions(filePath);
            AnonymousPermissionManager::instance()->restoreHomeDirectoryIfNoAnonymousShares();
        }

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
