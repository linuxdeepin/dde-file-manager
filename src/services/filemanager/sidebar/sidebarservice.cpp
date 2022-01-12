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

#include "dfm-base/utils/universalutils.h"

DSB_FM_BEGIN_NAMESPACE

namespace SideBar {

namespace DefaultGroup {
const char *const kCommon { "Common" };
const char *const kDevice { "Device" };
const char *const kBookmark { "Bookmark" };
const char *const kNetwork { "Network" };
const char *const kTag { "Tag" };
const char *const kOther { "Other" };
}   // namespace DefaultGroup

// custom event type
namespace EventType {
extern const int kEjectAction = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
};   // namespace EventType

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

/*!
 * \brief add a item to sidebar, like `DFMSideBarItemInterface` of old filemanager
 * \param info
 * \param cdFunc
 * \param menuFunc
 */
void SideBarService::addItem(const SideBar::ItemInfo &info, const SideBar::CdActionCallback &cdFunc,
                             const SideBar::ContextMenuCallback &menuFunc, const SideBar::RenameCallback &renameFunc)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, info, cdFunc, menuFunc, renameFunc);
}

/*!
 * \brief SideBarService::removeItem remove the item from sidebar.
 * \param url  key to find the item
 */
void SideBarService::removeItem(const QUrl &url)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, url);
}

void SideBarService::updateItem(const QUrl &url, const QString &newName, bool editable)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, url, newName, editable);
}

void SideBarService::insertItem(int index, const SideBar::ItemInfo &info, const SideBar::CdActionCallback &cdFunc, const SideBar::ContextMenuCallback &menuFunc, const SideBar::RenameCallback &renameFunc)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, index, info, cdFunc, menuFunc, renameFunc);
}
