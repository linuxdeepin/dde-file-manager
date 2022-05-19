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
#include "events/burneventreceiver.h"

#include "services/common/menu/menuservice.h"

#include <QWidget>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPBURN_NAMESPACE) };

bool Burn::start()
{
    bindEvent();
    DSC_USE_NAMESPACE
    MenuService::service()->registerScene(SendToDiscMenuCreator::name(), new SendToDiscMenuCreator);
    bindScene("SendToMenu");

    DiscStateManager::instance()->initilaize();

    return true;
}

void Burn::bindScene(const QString &parentScene)
{
    DSC_USE_NAMESPACE
    if (MenuService::service()->contains(parentScene)) {
        MenuService::service()->bind(SendToDiscMenuCreator::name(), parentScene);
    } else {
        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
            if (scene == parentScene)
                MenuService::service()->bind(SendToDiscMenuCreator::name(), scene);
        },
                Qt::DirectConnection);
    }
}

void Burn::bindEvent()
{
    auto type = DPF_EVENT_TYPE_SLOT(kCurrentEventSpace, "slot_ShowBurnDialog");
    Q_ASSERT(type > 0);
    dpfSlotChannel->connect(type, BurnEventReceiver::instance(), &BurnEventReceiver::handleShowBurnDlg);

    type = DPF_EVENT_TYPE_SLOT(kCurrentEventSpace, "slot_Erase");
    Q_ASSERT(type > 0);
    dpfSlotChannel->connect(type, BurnEventReceiver::instance(), &BurnEventReceiver::handleErase);

    type = DPF_EVENT_TYPE_SLOT(kCurrentEventSpace, "slot_PasteTo");
    Q_ASSERT(type > 0);
    dpfSlotChannel->connect(type, BurnEventReceiver::instance(), &BurnEventReceiver::handlePasteTo);
}
