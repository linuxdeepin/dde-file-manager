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
#ifndef TITLEBAR_DEFINES_H
#define TITLEBAR_DEFINES_H

#include "dfm_filemanager_service_global.h"

DSB_FM_BEGIN_NAMESPACE

namespace TitleBar {
namespace EventTopic {
extern const char *const kTitleBarOptionBtn;   // deifine "/org/deepin/event/titlebar/optionbutton"
}   // namespace EventTopic

namespace EventData {
extern const char *const kSwitchMode;
}   // namespace EventData

namespace EventProperty {
extern const char *const kWindowId;   // value is quint64
extern const char *const kViewMode;   // value is ViewMode
}   // namespace EventProperty

enum ViewMode {
    kIconMode = 0x01,
    kListMode = 0x02,
    kExtendMode = 0x04,
    AllViewMode = kIconMode | kListMode | kExtendMode
};

}   // namespace TitleBar

DSB_FM_END_NAMESPACE

#endif   // TITLEBAR_DEFINES_H
