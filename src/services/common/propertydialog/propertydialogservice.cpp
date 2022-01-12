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

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

PropertyDialogService::PropertyDialogService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<PropertyDialogService>()
{
}

void PropertyDialogService::addFileProperty(const QList<QUrl> &url)
{
    dpfInstance.eventUnicast().push(DSC_FUNC_NAME, url);
}

void PropertyDialogService::addDeviceProperty(const DeviceInfo &info)
{
    dpfInstance.eventUnicast().push(DSC_FUNC_NAME, info);
}

void PropertyDialogService::showTrashProperty(const QUrl &url)
{
    dpfInstance.eventUnicast().push(DSC_FUNC_NAME, url);
}

void PropertyDialogService::showComputerProperty(const QUrl &url)
{
    dpfInstance.eventUnicast().push(DSC_FUNC_NAME, url);
}
