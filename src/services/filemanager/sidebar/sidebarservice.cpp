/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "sidebarservice.h"
#include "sidebar_defines.h"
#include "private/sidebarservice_p.h"

DSB_FM_BEGIN_NAMESPACE

namespace SideBar {
namespace EventTopic {
const char *const kSideBar { "/org/deepin/event/sidebar" };
}   // namespace EventTopic

namespace EventData {
const char *const kCdAction { "CdAction" };
const char *const kContexMenu { "ContexMenu" };
const char *const kRename { "Rename" };
}   // namespace EventData

namespace EventProperty {
const char *const kWindowId { "WindowId" };
const char *const kUrl { "Url" };
const char *const kName { "Name" };
const char *const kPos { "Pos" };
}   // namespace EventProperty

namespace DefaultGroup {
const char *const kCommon { "Common" };
const char *const kDevice { "Device" };
const char *const kBookmark { "Bookmark" };
const char *const kNetwork { "Network" };
const char *const kTag { "Tag" };
const char *const kOther { "Other" };
}   // namespace DefaultGroup

}   // namespace Sidebar

DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE

/*!
 * \class SideBarServicePrivate
 * \brief
 */

SideBarServicePrivate::SideBarServicePrivate(SideBarService *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \class SideBarService
 * \brief
 */

SideBarService::SideBarService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<SideBarService>(),
      d(new SideBarServicePrivate(this))
{
}

SideBarService::~SideBarService()
{
}
