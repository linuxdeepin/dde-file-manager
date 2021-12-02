/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef CONTEXTS_H
#define CONTEXTS_H

#include "dfm_filemanager_service_global.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

namespace EventTypes {
const char *const kTopicWindowEvent = "WindowEvent";
const char *const kDataOpenNewWindow = "OpenNewWindow";
const char *const kPropertyKeyWindowIndex = "WindowIndex";

const char *const kSidebarContextMenuEvent = "SidebarContextMenuEvent";
const char *const kDataSidebarContextMenu = "SendSidebarContextMenuEvent";
const char *const kPropertySidebarItemUrl = "ItemUrl";
const char *const kPropertySidebarItemPos = "ItemPos";

const char *const kTopicSearchEvent = "SearchEvent";
const char *const kDataSearch = "Search";
const char *const kPropertyTargetUrl = "TargetUrl";
const char *const kPropertyKeyword = "Keyword";

const char *const kDataSetRootUrlEvent = "SetRootUrl";
const char *const kPropertyRootUrl = "RootUrl";
};   // namespace eventContainer

namespace SideBarGroups {
const char *const kCore = "core";
const char *const kDevice = "device";
}   // namespace eventContainer

DSB_FM_END_NAMESPACE

#endif   // CONTEXTS_H
