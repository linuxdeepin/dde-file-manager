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

#include "services/common/propertydialog/property_defines.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

#include <QWidget>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_computer {

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
    quint64 winId = FMWindowsIns.findWindowId(sender);

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
    u.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    u.setPath(path);
    cdTo(sender, u);
}

void ComputerEventCaller::cdTo(quint64 winId, const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url))
        return;

    DFMBASE_USE_NAMESPACE
    if (Application::appAttribute(Application::ApplicationAttribute::kAllwayOpenOnNewWindow).toBool())
        sendEnterInNewWindow(url);
    else
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, url);
}

void ComputerEventCaller::cdTo(quint64 winId, const QString &path)
{
    QUrl u;
    u.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    u.setPath(path);
    cdTo(winId, u);
}

void ComputerEventCaller::sendEnterInNewWindow(const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url))
        return;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, url);
}

void ComputerEventCaller::sendEnterInNewTab(quint64 winId, const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url))
        return;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewTab, winId, url);
}

void ComputerEventCaller::sendOpenItem(quint64 winId, const QUrl &url)
{
    dpfSignalDispatcher->publish(EventNameSpace::kComputerEventSpace, "signal_Operation_OpenItem", winId, url);
    qDebug() << "send open item: " << url;
}

void ComputerEventCaller::sendCtrlNOnItem(quint64 winId, const QUrl &url)
{
    dpfSignalDispatcher->publish(EventNameSpace::kComputerEventSpace, "signal_ShortCut_CtrlN", winId, url);
    qDebug() << "send ctrl n at item: " << url;
}

void ComputerEventCaller::sendCtrlTOnItem(quint64 winId, const QUrl &url)
{
    dpfSignalDispatcher->publish(EventNameSpace::kComputerEventSpace, "signal_ShortCut_CtrlT", winId, url);
    qDebug() << "send ctrl t at item: " << url;
}

void ComputerEventCaller::sendShowPropertyDialog(const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog, urls);
}

void ComputerEventCaller::sendErase(const QString &dev)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_Erase", dev);
}

}
