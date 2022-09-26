/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burn.h"
#include "menus/sendtodiscmenuscene.h"
#include "utils/discstatemanager.h"
#include "utils/burnhelper.h"
#include "utils/burnsignalmanager.h"
#include "events/burneventreceiver.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <QWidget>
#include <QTimer>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPBURN_NAMESPACE) };

void Burn::initialize()
{
    bindEvents();
    dpfSignalDispatcher->installEventFilter(GlobalEventType::kChangeCurrentUrl, this, &Burn::changeUrlEventFilter);
}

bool Burn::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(SendToDiscMenuCreator::name(), new SendToDiscMenuCreator);
    bindScene("ShareMenu");

    DiscStateManager::instance()->initilaize();

    connect(Application::dataPersistence(), &Settings::valueChanged, this, &Burn::onPersistenceDataChanged, Qt::DirectConnection);
    Application::dataPersistence()->removeGroup(Persistence::kBurnStateGroup);
    DevMngIns->startOpticalDiscScan();

    QString err;
    auto ret = DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.burn", &err);
    if (!ret)
        qWarning() << "create dconfig failed: " << err;

    return true;
}

void Burn::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(SendToDiscMenuCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Burn::bindSceneOnAdded);
    }
}

void Burn::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Burn::bindSceneOnAdded);
        bindScene(newScene);
    }
}

void Burn::bindEvents()
{
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_ShowBurnDialog", BurnEventReceiver::instance(), &BurnEventReceiver::handleShowBurnDlg);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Erase", BurnEventReceiver::instance(), &BurnEventReceiver::handleErase);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_PasteTo", BurnEventReceiver::instance(), &BurnEventReceiver::handlePasteTo);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_MountImage", BurnEventReceiver::instance(), &BurnEventReceiver::handleMountImage);

    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyResult, BurnEventReceiver::instance(), &BurnEventReceiver::handleCopyFilesResult);
}

bool Burn::changeUrlEventFilter(quint64 windowId, const QUrl &url)
{
    Q_UNUSED(windowId);
    if (url.scheme() == Global::Scheme::kBurn) {
        auto &&dev { BurnHelper::burnDestDevice(url) };
        if (DeviceUtils::isWorkingOpticalDiscDev(dev)) {
            emit BurnSignalManager::instance()->activeTaskDialog();
            return true;
        }
    }
    return false;
}

void Burn::onPersistenceDataChanged(const QString &group, const QString &key, const QVariant &value)
{
    if (group != Persistence::kBurnStateGroup)
        return;

    qInfo() << "Burn working state changed: " << key << value;
    auto &&map { value.toMap() };
    auto &&id { map[Persistence::kIdKey].toString() };
    auto &&working { map[Persistence::kWoringKey].toBool() };
    emit DevMngIns->opticalDiscWorkStateChanged(id, key, working);
}
