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

#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE
namespace TitleBar {
// custm event type
namespace EventType {
const int kSwitchMode = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kSettingsMenuTriggered = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kShowDetailView = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kDoSearch = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
};   // namespace EventType

}   // namespace TitleBar
DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE

bool TitleBarService::addCustomCrumbar(const TitleBar::CustomCrumb &info, const TitleBar::supportedUrlCallback &supportedUrlFunc, const TitleBar::seprateUrlCallback &seprateUrlFunc)
{
    return dpfInstance.eventUnicast().push(DSB_FUNC_NAME, info, supportedUrlFunc, seprateUrlFunc).toBool();
}

TitleBarService::TitleBarService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<TitleBarService>(),
      d(new TitleBarServicePrivate(this))
{
}

TitleBarServicePrivate::TitleBarServicePrivate(TitleBarService *serv)
    : QObject(nullptr), service(serv)
{
}

TitleBarService::~TitleBarService()
{
}
