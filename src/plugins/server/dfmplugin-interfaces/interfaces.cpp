/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "interfaces.h"

#include "deviceservice.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE

void Interfaces::initialize()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(DeviceService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Interfaces::start()
{
    auto &ctx = dpfInstance.serviceContext();
    DeviceService *service = ctx.service<DeviceService>(DeviceService::name());
    Q_ASSERT(service);
    service->startAutoMount();
    service->startMonitor();
    initDBusInterfaces();
    return true;
}

dpf::Plugin::ShutdownFlag Interfaces::stop()
{
    return Sync;
}

void Interfaces::initDBusInterfaces()
{

}
