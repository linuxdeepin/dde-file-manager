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
#ifndef SIDEBAR_DEFINES_H
#define SIDEBAR_DEFINES_H

#include "dfm_filemanager_service_global.h"

DSB_FM_BEGIN_NAMESPACE

namespace SideBar {
namespace EventTopic {
extern const char *const kSideBar;   // define "/org/deepin/event/sidebar"
extern const char *const kSideBarItem;   // define "/org/deepin/event/sidebar/item"
}   // namespace EventTopics

namespace EventData {
extern const char *const kCdAction;
extern const char *const kContexMenu;
extern const char *const kRename;
}   // namespace EventData

namespace EventProperty {
extern const char *const kWindowId;   // value is quint64
extern const char *const kUrl;   // value is Qurl
extern const char *const kName;   // value is QString
extern const char *const kPos;   // value is int
}   // namespace EventProperty
}   // namespace SideBar

DSB_FM_END_NAMESPACE

#endif   // SIDEBAR_DEFINES_H
