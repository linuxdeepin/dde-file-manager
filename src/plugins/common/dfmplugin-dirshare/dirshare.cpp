// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dirshare.h"
#include "dirsharemenu/dirsharemenuscene.h"
#include "widget/sharecontrolwidget.h"
#include "utils/usersharehelper.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>

#include <QLabel>
#include <QHBoxLayout>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

namespace dfmplugin_dirshare {
DFM_LOG_REISGER_CATEGORY(DPDIRSHARE_NAMESPACE)

void DirShare::initialize()
{
    UserShareHelperInstance;
    connect(UserShareHelperInstance, &UserShareHelper::shareAdded, this, &DirShare::onShareStateChanged);
    connect(UserShareHelperInstance, &UserShareHelper::shareRemoved, this, &DirShare::onShareStateChanged);

    bindEvents();
}

bool DirShare::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(DirShareMenuCreator::name(), new DirShareMenuCreator);

    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");

    CustomViewExtensionView func { DirShare::createShareControlWidget };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, "DirShare", 2);

    return true;
}

QWidget *DirShare::createShareControlWidget(const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    static QStringList supported { Global::Scheme::kFile, Global::Scheme::kUserShare };
    if (!supported.contains(url.scheme()))
        return nullptr;

    auto info = InfoFactory::create<FileInfo>(url);
    bool disableWidget = UserShareHelper::needDisableShareWidget(info);
    if (!UserShareHelper::canShare(info))
        return nullptr;

    return new ShareControlWidget(url, disableWidget);
}

void DirShare::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(DirShareMenuCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &DirShare::bindSceneOnAdded);
    }
}

void DirShare::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &DirShare::bindSceneOnAdded);
        bindScene(newScene);
    }
}

void DirShare::bindEvents()
{
    dpfSlotChannel->connect(kEventSpace, "slot_Share_StartSmbd", UserShareHelperInstance, &UserShareHelper::startSambaServiceAsync);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_IsSmbdRunning", UserShareHelperInstance, &UserShareHelper::isSambaServiceRunning);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_AddShare", UserShareHelperInstance, &UserShareHelper::share);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_RemoveShare", UserShareHelperInstance, &UserShareHelper::removeShareByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_IsPathShared", UserShareHelperInstance, &UserShareHelper::isShared);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_AllShareInfos", UserShareHelperInstance, &UserShareHelper::shareInfos);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareInfoOfFilePath", UserShareHelperInstance, &UserShareHelper::shareInfoByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareInfoOfShareName", UserShareHelperInstance, &UserShareHelper::shareInfoByShareName);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareNameOfFilePath", UserShareHelperInstance, &UserShareHelper::shareNameByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_WhoSharedByShareName", UserShareHelperInstance, &UserShareHelper::whoShared);

    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Share_SetPassword", UserShareHelperInstance, &UserShareHelper::handleSetPassword);
}

void DirShare::onShareStateChanged(const QString &path)
{
    QUrl url { QUrl::fromLocalFile(path) };
    if (!url.isValid())
        return;

    // request refresh emblem state
    auto eventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_canvas", "slot_FileInfoModel_UpdateFile") };
    if (eventID != DPF_NAMESPACE::EventTypeScope::kInValid)
        dpfSlotChannel->push("ddplugin_canvas", "slot_FileInfoModel_UpdateFile", url);
    else
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_FileUpdate", url);
}
}   // namespace dfmplugin_dirshare
