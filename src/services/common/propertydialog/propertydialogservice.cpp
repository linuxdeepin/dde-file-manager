/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "propertydialogservice.h"
#include "dfm-base/utils/universalutils.h"

DSC_BEGIN_NAMESPACE
namespace PropertyEventType {
const int  kEvokeTrashProperty = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int  kEvokeComputerProperty = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int  kEvokeDefaultFileProperty = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int  kEvokeDefaultDeviceProperty = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int  kEvokeCustomizeProperty = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}
DSC_END_NAMESPACE

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

PropertyDialogService::PropertyDialogService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<PropertyDialogService>()
{
}
