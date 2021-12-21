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

DSB_FM_BEGIN_NAMESPACE

namespace Computer {
namespace EventTopic {
const char *const kComputer { "computerEvent" };
}   // namespace EventTopic

namespace EventData {
const char *const kOpenDevice { "open" };
const char *const kOpenDeviceInNewWindow { "openInNewWindow" };
const char *const kOpenDeviceInNewTab { "openInNewTab" };
const char *const kShowDeviceProperty { "showProperty" };
const char *const kRenameDevice { "rename" };
const char *const kUnmountDevice { "unmount" };
const char *const kMountDevice { "mount" };
const char *const kSafelyRemoveDevice { "safelyRemove" };
const char *const kFormatDevice { "format" };

const char *const kAddDevice { "addDevice" };
const char *const kRemoveDevice { "removeDevice" };
const char *const kInsertDevice { "insertDevice" };
const char *const kAddGroup { "addGroup" };
const char *const kRemoveGroup { "removeGroup" };
}   // namespace EventData

namespace EventProperty {

}   // namespace EventProperty
}   // namespace Computer

ComputerService::ComputerService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<ComputerService>(),
      d(new ComputerServicePrivate(this))
{
}

ComputerService::~ComputerService()
{
}

ComputerServicePrivate::ComputerServicePrivate(dfm_service_filemanager::ComputerService *serv)
    : QObject(nullptr), service(serv)
{
}

DSB_FM_END_NAMESPACE
