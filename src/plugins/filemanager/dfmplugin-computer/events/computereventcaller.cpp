/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "computereventcaller.h"
#include "utils/computerutils.h"

#include "services/filemanager/computer/computer_defines.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/common/propertydialog/property_defines.h"
#include "services/common/burn/burn_defines.h"

#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/urlroute.h"

#include <dfm-framework/framework.h>

#include <QWidget>

DPCOMPUTER_BEGIN_NAMESPACE

/*!
 * \brief ComputerEventCaller::cdTo
 * \param sender
 * \param url  file:///path/to/the/directory
 */
void ComputerEventCaller::cdTo(QWidget *sender, const QUrl &url)
{
    if (!url.isValid())
        return;

    DSB_FM_USE_NAMESPACE
    auto &ctx = dpfInstance.serviceContext();
    auto windowServ = ctx.service<WindowsService>(WindowsService::name());
    quint64 winId = windowServ->findWindowId(sender);

    cdTo(winId, url);
}

/*!
 * \brief ComputerEventCaller::cdTo
 * \param sender
 * \param path /path/to/the/directory
 */
void ComputerEventCaller::cdTo(QWidget *sender, const QString &path)
{
    if (path.isEmpty())
        return;
    QUrl u;
    u.setScheme(dfmbase::SchemeTypes::kFile);
    u.setPath(path);
    cdTo(sender, u);
}

void ComputerEventCaller::cdTo(quint64 winId, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    dpfInstance.eventDispatcher().publish(GlobalEventType::kChangeCurrentUrl, winId, url);
}

void ComputerEventCaller::cdTo(quint64 winId, const QString &path)
{
    QUrl u;
    u.setScheme(dfmbase::SchemeTypes::kFile);
    u.setPath(path);
    cdTo(winId, u);
}

void ComputerEventCaller::sendEnterInNewWindow(const QUrl &url)
{
    dpfInstance.eventDispatcher().publish(dfmbase::GlobalEventType::kOpenNewWindow, url);
}

void ComputerEventCaller::sendEnterInNewTab(quint64 winId, const QUrl &url)
{
    dpfInstance.eventDispatcher().publish(dfmbase::GlobalEventType::kOpenNewTab, winId, url);
}

void ComputerEventCaller::sendContextActionTriggered(quint64 winId, const QUrl &url, const QString &action)
{
    dpfInstance.eventDispatcher().publish(DSB_FM_NAMESPACE::EventType::kContextActionTriggered, winId, url, action);
    qDebug() << "action triggered: " << url << action;
}

void ComputerEventCaller::sendOpenItem(quint64 winId, const QUrl &url)
{
    dpfInstance.eventDispatcher().publish(DSB_FM_NAMESPACE::EventType::kOnOpenItem, winId, url);
    qDebug() << "send open item: " << url;
}

void ComputerEventCaller::sendShowFilePropertyDialog(const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::PropertyEventType::kEvokeDefaultFileProperty, urls);
}

void ComputerEventCaller::sendShowDevicePropertyDialog(const DFMEntryFileInfoPointer &info)
{
    DSC_NAMESPACE::DeviceInfo devInfo;
    devInfo.icon = info->fileIcon();
    devInfo.deviceUrl = info->url();
    devInfo.deviceName = info->displayName();
    devInfo.deviceType = ComputerUtils::deviceTypeInfo(info);
    devInfo.fileSystem = info->extraProperty(GlobalServerDefines::DeviceProperty::kFileSystem).toString();
    devInfo.totalCapacity = info->sizeTotal();
    devInfo.availableSpace = info->sizeFree();

    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::PropertyEventType::kEvokeDefaultDeviceProperty, devInfo);
}

void ComputerEventCaller::sendErase(const QString &dev)
{
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::Burn::EventType::kErase, dev);
}

DPCOMPUTER_END_NAMESPACE
