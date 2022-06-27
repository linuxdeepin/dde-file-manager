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
#include "dirshare.h"
#include "sharemenu/sharemenuscene.h"
#include "widget/sharecontrolwidget.h"
#include "utils/usersharehelper.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"

#include <QLabel>
#include <QHBoxLayout>

using namespace dfmplugin_dirshare;
DSC_USE_NAMESPACE

void DirShare::initialize()
{
}

bool DirShare::start()
{
    UserShareHelperInstance;
    dfmplugin_menu_util::menuSceneRegisterScene(ShareMenuCreator::name(), new ShareMenuCreator);

    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");

    PropertyDialogService::service()->registerControlExpand(DirShare::createShareControlWidget, 2);

    bindEvents();
    return true;
}

dpf::Plugin::ShutdownFlag DirShare::stop()
{
    return kSync;
}

QWidget *DirShare::createShareControlWidget(const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    static QStringList supported { Global::Scheme::kFile, Global::Scheme::kUserShare };
    if (!supported.contains(url.scheme()))
        return nullptr;

    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (!UserShareHelper::canShare(info))
        return nullptr;

    return new ShareControlWidget(url);
}

void DirShare::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(ShareMenuCreator::name(), parentScene);
    } else {
        // todo(xst) menu signal_MenuScene_SceneAdded
        //        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
        //            if (scene == parentScene)
        //                MenuService::service()->bind(ShareMenuCreator::name(), scene);
        //        },
        //                Qt::DirectConnection);
    }
}

void DirShare::bindEvents()
{
    dpfSlotChannel->connect(kEventSpace, "slot_Share_StartSmbd", UserShareHelperInstance, &UserShareHelper::startSambaServiceAsync);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_IsSmbdRunning", UserShareHelperInstance, &UserShareHelper::isSambaServiceRunning);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_SetSmbPasswd", UserShareHelperInstance, &UserShareHelper::setSambaPasswd);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_AddShare", UserShareHelperInstance, &UserShareHelper::share);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_RemoveShare", UserShareHelperInstance, &UserShareHelper::removeShareByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_IsPathShared", UserShareHelperInstance, &UserShareHelper::isShared);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_AllShareInfos", UserShareHelperInstance, &UserShareHelper::shareInfos);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareInfoOfFilePath", UserShareHelperInstance, &UserShareHelper::shareInfoByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareInfoOfShareName", UserShareHelperInstance, &UserShareHelper::shareInfoByShareName);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_ShareNameOfFilePath", UserShareHelperInstance, &UserShareHelper::shareNameByPath);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_CurrentUserName", UserShareHelperInstance, &UserShareHelper::currentUserName);
    dpfSlotChannel->connect(kEventSpace, "slot_Share_WhoSharedByShareName", UserShareHelperInstance, &UserShareHelper::whoShared);
}
