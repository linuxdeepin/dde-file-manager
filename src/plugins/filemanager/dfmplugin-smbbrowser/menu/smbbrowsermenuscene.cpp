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
#include "smbbrowsermenuscene.h"
#include "smbintegration/smbintegrationmanager.h"
#include "private/smbbrowsermenuscene_p.h"

#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dfm_menu_defines.h"

#include <dfm-framework/dpf.h>

#include <QMenu>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *SmbBrowserMenuCreator::create()
{
    return new SmbBrowserMenuScene();
}

SmbBrowserMenuScene::SmbBrowserMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new SmbBrowserMenuScenePrivate(this))
{
}

SmbBrowserMenuScene::~SmbBrowserMenuScene()
{
}

QString SmbBrowserMenuScene::name() const
{
    return SmbBrowserMenuCreator::name();
}

bool SmbBrowserMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    if (d->selectFiles.count() != 1 || d->isEmptyArea)
        return false;

    return AbstractMenuScene::initialize(params);
}

bool SmbBrowserMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    auto act = parent->addAction(d->predicateName[SmbBrowserActionId::kOpenSmb]);
    act->setProperty(ActionPropertyKey::kActionID, SmbBrowserActionId::kOpenSmb);
    d->predicateAction[SmbBrowserActionId::kOpenSmb] = act;

    act = parent->addAction(d->predicateName[SmbBrowserActionId::kOpenSmbInNewWin]);
    act->setProperty(ActionPropertyKey::kActionID, SmbBrowserActionId::kOpenSmbInNewWin);
    d->predicateAction[SmbBrowserActionId::kOpenSmbInNewWin] = act;

    const QString &smbUrl = d->selectFiles.first().toString();
    if (SmbIntegrationManager::instance()->isSmbShareDirMounted(QUrl(smbUrl))) {
        act = parent->addAction(d->predicateName[SmbBrowserActionId::kUnmountSmb]);
        act->setProperty(ActionPropertyKey::kActionID, SmbBrowserActionId::kUnmountSmb);
        d->predicateAction[SmbBrowserActionId::kUnmountSmb] = act;
    } else {
        act = parent->addAction(d->predicateName[SmbBrowserActionId::kMountSmb]);
        act->setProperty(ActionPropertyKey::kActionID, SmbBrowserActionId::kMountSmb);
        d->predicateAction[SmbBrowserActionId::kMountSmb] = act;
    }

    return true;
}

void SmbBrowserMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
}

bool SmbBrowserMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actId))
        return false;

    if (d->selectFiles.count() != 1)
        return false;

    quint64 winId = d->windowId;
    const QString &smbUrl = d->selectFiles.first().toString();
    if (actId == SmbBrowserActionId::kOpenSmb || actId == SmbBrowserActionId::kMountSmb || actId == SmbBrowserActionId::kOpenSmbInNewWin) {
        DeviceManager::instance()->mountNetworkDeviceAsync(smbUrl, [actId, winId](bool ok, dfmmount::DeviceError err, const QString &mntPath) {
            if (!ok && err != DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) {
                DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
            } else {
                QUrl u = QUrl::fromLocalFile(mntPath);
                // If `smbUrl` is not mounted, the behavior of action `kMountSmb` is the same as `kOpenSmb`.
                if (actId == SmbBrowserActionId::kOpenSmb || actId == SmbBrowserActionId::kMountSmb)
                    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, u);
                else
                    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, u);
            }
        });
    } else if (actId == SmbBrowserActionId::kUnmountSmb) {
        const QStringList &smbMountPaths = SmbIntegrationManager::instance()->getSmbMountPathsByUrl(smbUrl);
        SmbIntegrationManager::instance()->actUmount(d->windowId, smbMountPaths);
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *SmbBrowserMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SmbBrowserMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

SmbBrowserMenuScenePrivate::SmbBrowserMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[SmbBrowserActionId::kOpenSmb] = tr("Open");
    predicateName[SmbBrowserActionId::kOpenSmbInNewWin] = tr("Open in new window");
    predicateName[SmbBrowserActionId::kMountSmb] = tr("Mount");
    predicateName[SmbBrowserActionId::kUnmountSmb] = tr("Unmount");
}
