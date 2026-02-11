// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfilewatcherprivate.h"
#include "fileutils/vaultfilewatcher.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultFileWatcherPrivate::VaultFileWatcherPrivate(const QUrl &fileUrl, VaultFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
    fmDebug() << "Vault: Creating file watcher private for URL:" << fileUrl.toString();
}

bool VaultFileWatcherPrivate::start()
{
    fmDebug() << "Vault: Starting file watcher";
    return proxyStaging && proxyStaging->startWatcher();
}

bool VaultFileWatcherPrivate::stop()
{
    fmDebug() << "Vault: Stopping file watcher";
    return proxyStaging && proxyStaging->stopWatcher();
}
