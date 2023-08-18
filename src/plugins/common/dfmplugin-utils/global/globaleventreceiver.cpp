// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaleventreceiver.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>

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

    auto fileInfo = InfoFactory::create<FileInfo>(url);
    if (fileInfo) {
        if (fileInfo->isAttributes(FileInfo::FileIsType::kIsDir)
                && fileInfo->isAttributes(FileInfo::FileIsType::kIsSymLink)) {
            localPath = fileInfo->urlOf(FileInfo::FileUrlInfoType::kRedirectedFileUrl).toLocalFile();
        }
    }

    QProcess::startDetached("dde-file-manager-pkexec", { localPath });
}
