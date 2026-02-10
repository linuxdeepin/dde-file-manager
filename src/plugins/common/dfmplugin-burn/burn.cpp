// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burn.h"
#include "menus/sendtodiscmenuscene.h"
#include "utils/discstatemanager.h"
#include "utils/burnhelper.h"
#include "utils/burnsignalmanager.h"
#include "events/burneventreceiver.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QWidget>
#include <QTimer>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_burn {

DFM_LOG_REGISTER_CATEGORY(DPBURN_NAMESPACE)

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
        fmWarning() << "create dconfig failed: " << err;

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
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_BurnDialog_Show", BurnEventReceiver::instance(), &BurnEventReceiver::handleShowBurnDlg);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_DumpISODialog_Show", BurnEventReceiver::instance(), &BurnEventReceiver::handleShowDumpISODlg);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Erase", BurnEventReceiver::instance(), &BurnEventReceiver::handleErase);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_PasteTo", BurnEventReceiver::instance(), &BurnEventReceiver::handlePasteTo);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_MountImage", BurnEventReceiver::instance(), &BurnEventReceiver::handleMountImage);

    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyResult, BurnEventReceiver::instance(), &BurnEventReceiver::handleCopyFilesResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult, BurnEventReceiver::instance(), &BurnEventReceiver::handleFileCutResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult, BurnEventReceiver::instance(), &BurnEventReceiver::handleFileRemoveResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, BurnEventReceiver::instance(), &BurnEventReceiver::handleFileRenameResult);
}

bool Burn::changeUrlEventFilter(quint64 windowId, const QUrl &url)
{
    Q_UNUSED(windowId);
    if (url.scheme() == Global::Scheme::kBurn) {
        const auto &dev { BurnHelper::burnDestDevice(url) };
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

    fmInfo() << "Burn working state changed: " << key << value;
    auto &&map { value.toMap() };
    auto &&id { map[Persistence::kIdKey].toString() };
    auto &&working { map[Persistence::kWoringKey].toBool() };
    emit DevMngIns->opticalDiscWorkStateChanged(id, key, working);
}

}   // namespace dfmplugin_burn
