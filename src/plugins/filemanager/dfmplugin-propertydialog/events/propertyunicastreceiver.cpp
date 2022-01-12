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
#include "propertyunicastreceiver.h"
#include "utils/filepropertydialogmanager.h"

#include <dfm-framework/framework.h>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

/*!
 * \brief topic is defined in SideBarService
 * \param func
 * \return
 */
inline QString topic(const QString &func)
{
    return QString(STR2(DSC_NAMESPACE)) + "::" + func;
}

PropertyUnicastReceiver::PropertyUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}

PropertyUnicastReceiver *PropertyUnicastReceiver::instance()
{
    static PropertyUnicastReceiver receiver;
    return &receiver;
}

void PropertyUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("PropertyDialogService::addFileProperty"), this, &PropertyUnicastReceiver::addFilePropertyControl);
    dpfInstance.eventUnicast().connect(topic("PropertyDialogService::addDeviceProperty"), this, &PropertyUnicastReceiver::addDeviceProperty);
    dpfInstance.eventUnicast().connect(topic("PropertyDialogService::showTrashProperty"), this, &PropertyUnicastReceiver::showTrashProperty);
    dpfInstance.eventUnicast().connect(topic("PropertyDialogService::showComputerProperty"), this, &PropertyUnicastReceiver::showComputerProperty);
}

void PropertyUnicastReceiver::addFilePropertyControl(const QList<QUrl> &url)
{
    FilePropertyDialogManager *fileDialogManager = FilePropertyDialogManager::instance();
    fileDialogManager->showFilePropertyDialog(url);
}

void PropertyUnicastReceiver::addDeviceProperty(const DeviceInfo &info)
{
    FilePropertyDialogManager *fileDialogManager = FilePropertyDialogManager::instance();
    fileDialogManager->showDevicePropertyDialog(info);
}

void PropertyUnicastReceiver::showTrashProperty(const QUrl &url)
{
    FilePropertyDialogManager *fileDialogManager = FilePropertyDialogManager::instance();
    fileDialogManager->showTrashPropertyDialog();
}

void PropertyUnicastReceiver::showComputerProperty(const QUrl &url)
{
    FilePropertyDialogManager *fileDialogManager = FilePropertyDialogManager::instance();
    fileDialogManager->showComputerPropertyDialog();
}
