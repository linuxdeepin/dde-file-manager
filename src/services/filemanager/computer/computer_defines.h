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
#ifndef COMPUTER_DEFINES_H
#define COMPUTER_DEFINES_H

#include "dfm_filemanager_service_global.h"

DSB_FM_BEGIN_NAMESPACE

namespace Computer {
namespace EventTopic {
extern const char *const kComputer;
}   // namespace EventTopic

namespace EventData {
extern const char *const kOpenDevice;
extern const char *const kOpenDeviceInNewWindow;
extern const char *const kOpenDeviceInNewTab;
extern const char *const kShowDeviceProperty;
extern const char *const kRenameDevice;
extern const char *const kUnmountDevice;
extern const char *const kMountDevice;
extern const char *const kSafelyRemoveDevice;
extern const char *const kFormatDevice;

extern const char *const kAddDevice;
extern const char *const kRemoveDevice;
extern const char *const kInsertDevice;
extern const char *const kAddGroup;
extern const char *const kRemoveGroup;
}   // namespace EventData

namespace EventProperty {

}   // namespace EventProperty
}   // namespace Computer

DSB_FM_END_NAMESPACE

#endif   // COMPUTER_DEFINES_H
