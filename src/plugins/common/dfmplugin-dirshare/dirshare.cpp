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
#include "dirsharemenu/dirsharemenuscene.h"
#include "widget/sharecontrolwidget.h"
#include "utils/usersharehelper.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"

#include <QLabel>
#include <QHBoxLayout>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

using namespace dfmplugin_dirshare;

void DirShare::initialize()
{
}

bool DirShare::start()
{
    UserShareHelperInstance;
    dfmplugin_menu_util::menuSceneRegisterScene(DirShareMenuCreator::name(), new DirShareMenuCreator);

    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");

    CustomViewExtensionView func { DirShare::createShareControlWidget };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, "DirShare", 2);

    bindEvents();
    return true;
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
