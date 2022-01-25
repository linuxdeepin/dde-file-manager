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
#include "computerunicastreceiver.h"
#include "watcher/computeritemwatcher.h"

#include "services/filemanager/dfm_filemanager_service_global.h"

DPCOMPUTER_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)
inline QString topic(const QString &func)
{
    return QString(STR2(DSB_FM_NAMESPACE)) + "::" + func;
}

/*!
 * \class ComputerUnicastReceiver
 * \brief for handle events from computer service
 */

ComputerUnicastReceiver::ComputerUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}

ComputerUnicastReceiver *ComputerUnicastReceiver::instance()
{
    static ComputerUnicastReceiver receiver;
    return &receiver;
}

void ComputerUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("ComputerService::addDevice"), this, &ComputerUnicastReceiver::doAddDevice);
    dpfInstance.eventUnicast().connect(topic("ComputerService::removeDevice"), this, &ComputerUnicastReceiver::doRemoveDevice);
}

void ComputerUnicastReceiver::doAddDevice(const QString &name, const QUrl &url)
{
    ComputerItemWatcherInstance->addDevice(name, url);
}

void ComputerUnicastReceiver::doRemoveDevice(const QUrl &url)
{
    ComputerItemWatcherInstance->removeDevice(url);
}
