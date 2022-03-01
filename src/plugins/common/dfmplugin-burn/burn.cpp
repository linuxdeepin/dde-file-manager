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

#include "events/burneventreceiver.h"

#include <QWidget>

DPBURN_USE_NAMESPACE

void burn::initialize()
{
}

bool burn::start()
{
    DSC_USE_NAMESPACE

    dpfInstance.eventDispatcher().subscribe(Burn::EventType::kShowBurnDlg,
                                            BurnEventReceiver::instance(),
                                            &BurnEventReceiver::handleShowBurnDlg);
    dpfInstance.eventDispatcher().subscribe(Burn::EventType::kErase,
                                            BurnEventReceiver::instance(),
                                            &BurnEventReceiver::handleErase);
    return true;
}

dpf::Plugin::ShutdownFlag burn::stop()
{
    return kSync;
}
