/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "computereventhandler.h"
#include "services/filemanager/computer/computer_defines.h"

DPCOMPUTER_BEGIN_NAMESPACE

using namespace DSB_FM_NAMESPACE::Computer;
ComputerEventHandler::ComputerEventHandler(QObject *parent)
    : dpf::EventHandler(parent),
      AutoEventHandlerRegister<ComputerEventHandler>()
{
}

dpf::EventHandler::Type ComputerEventHandler::type()
{
    return EventHandler::Type::Sync;
}

QStringList ComputerEventHandler::topics()
{
    return QStringList() << EventTopic::kComputer;
}

void ComputerEventHandler::eventProcess(const dpf::Event &event)
{
    if (event.topic() == EventTopic::kComputer) {
        const auto &data = event.data().toString();
        qDebug() << "computer event is handled: " << data;
        if (data == EventData::kOpenDevice) {
        } else if (data == EventData::kOpenDeviceInNewWindow) {
        } else if (data == EventData::kOpenDeviceInNewTab) {
        } else if (data == EventData::kShowDeviceProperty) {
        } else if (data == EventData::kRenameDevice) {
        } else if (data == EventData::kUnmountDevice) {
        } else if (data == EventData::kMountDevice) {
        } else if (data == EventData::kSafelyRemoveDevice) {
        } else if (data == EventData::kFormatDevice) {
        } else if (data == EventData::kAddDevice) {
        } else if (data == EventData::kRemoveDevice) {
        } else if (data == EventData::kInsertDevice) {
        } else if (data == EventData::kAddGroup) {
        } else if (data == EventData::kRemoveGroup) {
        } else {
            qDebug() << "unexpected event data: " << event.data();
        }
    } else {
        qWarning() << "unexcepted event topic: " << event.topic();
        abort();
    }
}

DPCOMPUTER_END_NAMESPACE
