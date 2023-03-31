// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaleventreceiver.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QProcess>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

GlobalEventReceiver::GlobalEventReceiver(QObject *parent)
    : QObject(parent)
{
}

GlobalEventReceiver::~GlobalEventReceiver()
{
}

void GlobalEventReceiver::initEventConnect()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenAsAdmin,
                                   this, &GlobalEventReceiver::handleOpenAsAdmin);
}

void GlobalEventReceiver::handleOpenAsAdmin(const QUrl &url)
{
    if (url.isEmpty() || !url.isValid()) {
        qWarning() << "Invalid Url: " << url;
        return;
    }

    QString localPath { url.toLocalFile() };
    if (!QDir(localPath).exists()) {
        qWarning() << "Url path not exists: " << localPath;
        return;
    }

    QProcess::startDetached("dde-file-manager-pkexec", { localPath });
}
