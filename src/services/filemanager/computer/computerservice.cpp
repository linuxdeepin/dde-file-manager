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
#include "computerservice.h"
#include "private/computerservice_p.h"
#include "computer_defines.h"

#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QUrl>

DSB_FM_BEGIN_NAMESPACE

// custom event type
namespace EventType {
const int kContextActionTriggered = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kOnOpenItem = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
};   // namespace EventType

ComputerService::ComputerService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<ComputerService>(),
      d(new ComputerServicePrivate(this))
{
}

ComputerService::~ComputerService()
{
}

void ComputerService::addDevice(const QString &groupName, const QUrl &url)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, groupName, url);
}

void ComputerService::removeDevice(const QUrl &url)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, url);
}

ComputerServicePrivate::ComputerServicePrivate(dfm_service_filemanager::ComputerService *serv)
    : QObject(nullptr), service(serv)
{
}

DSB_FM_END_NAMESPACE
