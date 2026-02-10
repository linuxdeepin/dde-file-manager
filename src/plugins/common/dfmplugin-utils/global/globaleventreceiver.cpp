// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
        fmWarning() << "Invalid Url: " << url;
        return;
    }

    auto fileInfo = InfoFactory::create<FileInfo>(url);
    if (!fileInfo)
        fmWarning() << "Cannot create fileinfo for url: " << url;

    QString openPath { url.toLocalFile() };
    if (fileInfo && fileInfo->isAttributes(FileInfo::FileIsType::kIsDir)
        && fileInfo->isAttributes(FileInfo::FileIsType::kIsSymLink)) {
        openPath = fileInfo->urlOf(FileInfo::FileUrlInfoType::kRedirectedFileUrl).toLocalFile();
    }

    if (openPath.isEmpty())
        openPath = url.toString();

    QProcess::startDetached("dde-file-manager-pkexec", { openPath });
}
