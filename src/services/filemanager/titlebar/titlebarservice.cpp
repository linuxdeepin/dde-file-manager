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
#include "titlebarservice.h"
#include "titlebar_defines.h"
#include "private/titlebarservice_p.h"

DSB_FM_BEGIN_NAMESPACE
namespace TitleBar {
namespace EventTopic {
const char *const kTitleBar { "/org/deepin/event/titlebar" };
}   // namespace EventTopic

namespace EventData {
const char *const kSwitchMode { "SwitchMode" };
const char *const kSettingsMenuTriggered { "SettingsMenuTriggered" };
}   // namespace EventData

namespace EventProperty {
const char *const kWindowId { "WindowId" };
const char *const kViewMode { "ViewMode" };
const char *const kMenuAction { "MenuAction" };
}   // namespace EventProperty
}   // namespace TitleBar
DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE

TitleBarServicePrivate::TitleBarServicePrivate(TitleBarService *serv)
    : QObject(nullptr), service(serv)
{
}

TitleBarService::TitleBarService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<TitleBarService>(),
      d(new TitleBarServicePrivate(this))
{
}

TitleBarService::~TitleBarService()
{
}
