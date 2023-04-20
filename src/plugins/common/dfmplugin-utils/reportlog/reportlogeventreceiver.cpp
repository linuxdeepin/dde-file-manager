// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogeventreceiver.h"
#include "rlog/rlog.h"
#include "rlog/datas/smbreportdata.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/private/devicehelper.h>

#include <dfm-framework/dpf.h>

#include <dfm-mount/dblockdevice.h>

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
    connect(DeviceManager::instance(), &DeviceManager::blockDevMountResult,
            this, &ReportLogEventReceiver::handleBlockMountData);

    // connect all the signal events of plugins which need report log.
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);

    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("ddplugin_organizer", "signal_CollectionView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);

    lazyBindCommitEvent("dfmplugin-search", "dfmplugin_search");
    lazyBindCommitEvent("dfmplugin-vault", "dfmplugin_vault");

    lazyBindMenuDataEvent("dfmplugin-myshares", "dfmplugin_myshares");
    lazyBindMenuDataEvent("dfmplugin-smbbrowser", "dfmplugin_smbbrowser");
    lazyBindMenuDataEvent("dfmplugin-vault", "dfmplugin_vault");
    lazyBindMenuDataEvent("dfmplugin-trash", "dfmplugin_trash");
    lazyBindMenuDataEvent("dfmplugin-recent", "dfmplugin_recent");
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

void ReportLogEventReceiver::lazyBindCommitEvent(const QString &plugin, const QString &space)
{
    auto pluginName { DPF_NAMESPACE::LifeCycle::pluginMetaObj(plugin) };
    if (pluginName && pluginName->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        dpfSignalDispatcher->subscribe(space, "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    } else {
        connect(DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [=](const QString &iid, const QString &name) {
            Q_UNUSED(iid)
            if (name == plugin)
                dpfSignalDispatcher->subscribe(space, "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
        },
                Qt::DirectConnection);
    }
}

void ReportLogEventReceiver::lazyBindMenuDataEvent(const QString &plugin, const QString &space)
{
    auto pluginName { DPF_NAMESPACE::LifeCycle::pluginMetaObj(plugin) };
    if (pluginName && pluginName->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        dpfSignalDispatcher->subscribe(space, "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    } else {
        connect(DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [=](const QString &iid, const QString &name) {
            Q_UNUSED(iid)
            if (name == plugin)
                dpfSignalDispatcher->subscribe(space, "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
        },
                Qt::DirectConnection);
    }
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
            auto info = InfoFactory::create<FileInfo>(url);
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

void ReportLogEventReceiver::handleBlockMountData(const QString& id, bool result)
{
    if (id.isEmpty()) {
        qDebug() << "Can't report empty devices' operation";
        return;
    }

    QVariantMap rec {};
    if (result) {
        BlockDevAutoPtr device = DeviceHelper::createBlockDevice(id);

        if (device.isNull()) {
            qDebug() << "Can't report unexist devices' operation";
            return;
        }

        rec.insert("fileSystem", device->fileSystem());
        rec.insert("standardSize", device->sizeTotal());
        rec.insert("mountResult", result);
    } else {
        rec.insert("fileSystem", "unknown");
        rec.insert("standardSize", 0);
        rec.insert("mountResult", result);
    }

    qInfo() << "rlog: mount result: " << rec;
    RLog::instance()->commit("BlockMount", rec);
}
