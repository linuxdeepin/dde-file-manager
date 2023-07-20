// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualreportlogplugin.h"
#include "reportlogmanager.h"

#include <QTimer>

using namespace dfmplugin_utils;

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

        ReportLogManager::instance()->commit("AppStartup", data);
    });
    return true;
}
