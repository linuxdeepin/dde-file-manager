// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computereventcaller.h"
#include "utils/computerutils.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

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
    cdTo(sender, ComputerUtils::makeLocalUrl(path));
}

void ComputerEventCaller::cdTo(quint64 winId, const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url)) {
        qInfo() << "gvfs url not exists" << url;
        return;
    }

    DFMBASE_USE_NAMESPACE
    if (Application::appAttribute(Application::ApplicationAttribute::kAllwayOpenOnNewWindow).toBool())
        sendEnterInNewWindow(url);
    else
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, url);
}

void ComputerEventCaller::cdTo(quint64 winId, const QString &path)
{
    if (path.isEmpty())
        return;
    cdTo(winId, ComputerUtils::makeLocalUrl(path));
}

void ComputerEventCaller::sendEnterInNewWindow(const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url)) {
        qInfo() << "gvfs url not exists" << url;
        return;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, url);
}

void ComputerEventCaller::sendEnterInNewTab(quint64 winId, const QUrl &url)
{
    if (!ComputerUtils::checkGvfsMountExist(url)) {
        qInfo() << "gvfs url not exists" << url;
        return;
    }

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
    qDebug() << "send ctrl N at item: " << url;
}

void ComputerEventCaller::sendCtrlTOnItem(quint64 winId, const QUrl &url)
{
    dpfSignalDispatcher->publish(EventNameSpace::kComputerEventSpace, "signal_ShortCut_CtrlT", winId, url);
    qDebug() << "send ctrl T at item: " << url;
}

void ComputerEventCaller::sendShowPropertyDialog(const QList<QUrl> &urls)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void ComputerEventCaller::sendErase(const QString &dev)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_Erase", dev);
}

}
