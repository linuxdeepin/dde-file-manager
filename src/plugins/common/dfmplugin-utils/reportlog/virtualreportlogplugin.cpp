// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualreportlogplugin.h"
#include "reportlogmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QTimer>

using namespace dfmbase;
using namespace dfmplugin_utils;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

void VirtualReportLogPlugin::initialize()
{
    ReportLogManager::instance()->init();

    eventReceiver->bindEvents();
}

bool VirtualReportLogPlugin::start()
{
    QTimer::singleShot(3000, this, [=]() {
        QVariantMap data;
        data.insert("type", true);
        if (qAppName() == "dde-file-manager")
            data.insert("TreeViewModeEnabled", DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool());

        ReportLogManager::instance()->commit("AppStartup", data);
    });
    return true;
}
