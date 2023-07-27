// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfilewatcherprivate.h"
#include "fileutils/vaultfilewatcher.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultFileWatcherPrivate::VaultFileWatcherPrivate(const QUrl &fileUrl, VaultFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool VaultFileWatcherPrivate::start()
{
    return proxyStaging && proxyStaging->startWatcher();
}

bool VaultFileWatcherPrivate::stop()
{
    return proxyStaging && proxyStaging->stopWatcher();
}
