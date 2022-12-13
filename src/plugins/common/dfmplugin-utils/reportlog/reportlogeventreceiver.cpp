/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
 *
 * Maintainer: zhuangshu<zhuangshu@uniontech.com>
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
#include "reportlogeventreceiver.h"
#include "rlog/rlog.h"
#include "rlog/datas/smbreportdata.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/devicemanager.h"

#include <dfm-framework/dpf.h>

#include <QApplication>

using namespace dfmbase;
using namespace dfmplugin_utils;

void ReportLogEventReceiver::bindEvents()
{
    connect(qApp, &QApplication::aboutToQuit, this, [=]() {
        // report quit
        QVariantMap data;
        data.insert("type", false);
        RLog::instance()->commit("AppStartup", data);
    });

    // connect device manager mount result signal
    connect(DeviceManager::instance(), &DeviceManager::mountNetworkDeviceResult,
            this, &ReportLogEventReceiver::handleMountNetworkResult);

    // connect all the signal events of plugins which need report log.
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    dpfSignalDispatcher->subscribe("dfmplugin_search", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    dpfSignalDispatcher->subscribe("dfmplugin_vault", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);

    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("ddplugin_organizer", "signal_CollectionView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_myshares", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_smbbrowser", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_vault", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_trash", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_recent", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
}

void ReportLogEventReceiver::handleMountNetworkResult(bool ret, dfmmount::DeviceError err, const QString &msg)
{
    using namespace dfmmount;

    QVariantMap data;
    data.insert("result", ret);

    if (!ret) {
        switch (err) {
        case DeviceError::kUserErrorUserCancelled:
            data.insert("errorId", SmbReportData::kUserCancelError);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", "User cancel mount dialog.");
            break;
        case DeviceError::kUDisksErrorNotMounted:
        case DeviceError::kGIOErrorNotMounted:
        case DeviceError::kUserErrorNotMounted:
            data.insert("errorId", SmbReportData::kNotMount);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", msg);
            break;
        default:
            data.insert("errorId", SmbReportData::kMountError);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", msg);
            break;
        }
    }

    RLog::instance()->commit("Smb", data);
}

ReportLogEventReceiver::ReportLogEventReceiver(QObject *parent)
    : QObject(parent)
{
}

void ReportLogEventReceiver::commit(const QString &type, const QVariantMap &args)
{
    RLog::instance()->commit(type, args);
}

void ReportLogEventReceiver::handleMenuData(const QString &name, const QList<QUrl> &urlList)
{
    QVariantMap data {};
    data.insert("item_name", name);

    QString location("");
    QStringList types {};

    if (urlList.count() > 0) {
        location = "File";

        for (auto url : urlList) {
            auto info = InfoFactory::create<AbstractFileInfo>(url);
            if (info) {
                QString type = info->nameOf(NameInfoType::kMimeTypeName);
                if (!types.contains(type))
                    types << type;
            }
        }

    } else {
        location = "Workspace";
    }

    data.insert("location", location);
    data.insert("type", types);

    RLog::instance()->commit("FileMenu", data);
}
