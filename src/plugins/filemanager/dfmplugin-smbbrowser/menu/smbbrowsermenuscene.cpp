// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbrowsermenuscene.h"
#include "actioniddefines.h"
#include "events/smbbrowsereventcaller.h"
#include "private/smbbrowsermenuscene_p.h"
#include "utils/smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QDir>

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

    d->url = d->selectFiles.first();

    auto subScenes = subscene();
    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene("DConfigMenuFilter"))
        subScenes << filterScene;
    setSubscene(subScenes);

    return AbstractMenuScene::initialize(params);
}

bool SmbBrowserMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    auto addAct = [&](const QString &actId) {
        auto act = parent->addAction(d->predicateName[actId]);
        act->setProperty(ActionPropertyKey::kActionID, actId);
        d->predicateAction[actId] = act;
    };

    addAct(SmbBrowserActionId::kOpenSmb);
    addAct(SmbBrowserActionId::kOpenSmbInNewWin);
    addAct(SmbBrowserActionId::kOpenSmbInNewTab);
    addAct(SmbBrowserActionId::kMountSmb);
    addAct(SmbBrowserActionId::kUnmountSmb);
    addAct(SmbBrowserActionId::kProperties);

    return true;
}

void SmbBrowserMenuScene::updateState(QMenu *parent)
{
    auto mountAct = d->predicateAction[SmbBrowserActionId::kMountSmb];
    auto unmountAct = d->predicateAction[SmbBrowserActionId::kUnmountSmb];
    auto propertyAct = d->predicateAction[SmbBrowserActionId::kProperties];

    if (!(mountAct && unmountAct && propertyAct))
        return AbstractMenuScene::updateState(parent);

    bool isMounted = smb_browser_utils::isSmbMounted(d->url.toString());
    mountAct->setVisible(!isMounted);
    unmountAct->setVisible(isMounted);
    propertyAct->setEnabled(isMounted);

    AbstractMenuScene::updateState(parent);
}

bool SmbBrowserMenuScene::triggered(QAction *action)
{
    if (!action)
        return AbstractMenuScene::triggered(action);

    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actId))
        return AbstractMenuScene::triggered(action);

    if (d->selectFiles.count() != 1)
        return AbstractMenuScene::triggered(action);

    quint64 winId = d->windowId;
    const QString &smbUrl = d->selectFiles.first().toString();
    if (actId == SmbBrowserActionId::kOpenSmb)
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, QUrl(smbUrl));
    else if (actId == SmbBrowserActionId::kOpenSmbInNewTab)
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, winId, QUrl(smbUrl));
    else if (actId == SmbBrowserActionId::kOpenSmbInNewWin)
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, QUrl(smbUrl));
    else if (actId == SmbBrowserActionId::kMountSmb)
        d->actMount();
    else if (actId == SmbBrowserActionId::kUnmountSmb)
        d->actUnmount();
    else if (actId == SmbBrowserActionId::kProperties)
        d->actProperties();
    else
        return AbstractMenuScene::triggered(action);
    return true;
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
    predicateName[SmbBrowserActionId::kOpenSmbInNewTab] = tr("Open in new tab");
    predicateName[SmbBrowserActionId::kProperties] = tr("Properties");
    predicateName[SmbBrowserActionId::kMountSmb] = tr("Mount");
    predicateName[SmbBrowserActionId::kUnmountSmb] = tr("Unmount");
}

void SmbBrowserMenuScenePrivate::actUnmount()
{
    const QString &smbPath = url.toString().toLower();
    const QString &devId = smb_browser_utils::getDeviceIdByStdSmb(smbPath);
    qDebug() << "get device id of" << url << devId;

    DeviceManager::instance()->unmountProtocolDevAsync(devId, {}, [](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
        if (!ok)
            DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err);
    });
}

void SmbBrowserMenuScenePrivate::actMount()
{
    const QString &smbPath = url.toString().toLower();
    DeviceManager::instance()->mountNetworkDeviceAsync(smbPath, [smbPath](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
        qInfo() << "mount done: " << smbPath << ok << err.code << err.message << mpt;
        if (!ok && err.code != DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted)
            DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err);
    });
}

void SmbBrowserMenuScenePrivate::actProperties()
{
    const QString &smbPath = url.toString().toLower();
    const QString &devId = smb_browser_utils::getDeviceIdByStdSmb(smbPath);

    QUrl protoEntryUrl;
    protoEntryUrl.setScheme("entry");
    protoEntryUrl.setPath(devId + ".protodev");   // use computer item to show property

    QList<QUrl> urls { protoEntryUrl };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}
