// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogeventreceiver.h"
#include "reportlogmanager.h"

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
        ReportLogManager::instance()->commit("AppStartup", data);
    });

    // connect device manager mount result signal
    connect(DeviceManager::instance(), &DeviceManager::mountNetworkDeviceResult,
            this, &ReportLogEventReceiver::handleMountNetworkResult);
    connect(DeviceManager::instance(), &DeviceManager::blockDevMountResult,
            this, &ReportLogEventReceiver::handleBlockMountData);

    // connect all the signal events of plugins which need report log.
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    auto canvasEventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_canvas", "signal_CanvasView_ReportMenuData") };
    if (canvasEventID != DPF_NAMESPACE::EventTypeScope::kInValid) {
        dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
        dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_ReportLog_LoadFilesFinish", this, &ReportLogEventReceiver::handleDesktopStartupData);
    }
    auto organizerEventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_organizer", "signal_CollectionView_ReportMenuData") };
    if (organizerEventID != DPF_NAMESPACE::EventTypeScope::kInValid)
        dpfSignalDispatcher->subscribe("ddplugin_organizer", "signal_CollectionView_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);

    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_ReportLog_MenuData", this, &ReportLogEventReceiver::handleMenuData);

    auto backgroundEventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_background", "signal_ReportLog_BackgroundPaint") };
    if (backgroundEventID != DPF_NAMESPACE::EventTypeScope::kInValid)
        dpfSignalDispatcher->subscribe("ddplugin_background", "signal_ReportLog_BackgroundPaint", this, &ReportLogEventReceiver::handleDesktopStartupData);


    lazyBindCommitEvent("dfmplugin-search", "dfmplugin_search");
    lazyBindCommitEvent("dfmplugin-vault", "dfmplugin_vault");

    lazyBindMenuDataEvent("dfmplugin-myshares", "dfmplugin_myshares");
    lazyBindMenuDataEvent("dfmplugin-smbbrowser", "dfmplugin_smbbrowser");
    lazyBindMenuDataEvent("dfmplugin-vault", "dfmplugin_vault");
    lazyBindMenuDataEvent("dfmplugin-trash", "dfmplugin_trash");
    lazyBindMenuDataEvent("dfmplugin-recent", "dfmplugin_recent");
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
    ReportLogManager::instance()->commit(type, args);
}

void ReportLogEventReceiver::handleMenuData(const QString &name, const QList<QUrl> &urlList)
{
    ReportLogManager::instance()->reportMenuData(name, urlList);
}

void ReportLogEventReceiver::handleBlockMountData(const QString &id, bool result)
{
    ReportLogManager::instance()->reportBlockMountData(id, result);
}

void ReportLogEventReceiver::handleMountNetworkResult(const QString &, bool ret, dfmmount::DeviceError err, const QString &msg)
{
    ReportLogManager::instance()->reportNetworkMountData(ret, err, msg);
}

void ReportLogEventReceiver::handleDesktopStartupData(const QString &key, const QVariant &data)
{
    ReportLogManager::instance()->reportDesktopStartUp(key, data);
}
