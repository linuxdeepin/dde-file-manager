// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfilewatcher.h"
#include "utils/vaulthelper.h"
#include "private/vaultfilewatcherprivate.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultFileWatcher::VaultFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new VaultFileWatcherPrivate(VaultHelper::vaultToLocalUrl(url), this), parent)
{
    fmDebug() << "Vault: Creating file watcher for URL:" << url.toString();
    dptr = dynamic_cast<VaultFileWatcherPrivate *>(d.data());
    QUrl localUrl = VaultHelper::vaultToLocalUrl(url);
    dptr->proxyStaging = WatcherFactory::create<AbstractFileWatcher>(localUrl);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &VaultFileWatcher::onFileAttributeChanged, Qt::QueuedConnection);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileDeleted,
            this, &VaultFileWatcher::onFileDeleted, Qt::QueuedConnection);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileRename,
            this, &VaultFileWatcher::onFileRename, Qt::QueuedConnection);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::subfileCreated,
            this, &VaultFileWatcher::onSubfileCreated, Qt::QueuedConnection);
    fmDebug() << "Vault: File watcher created successfully";
}

VaultFileWatcher::~VaultFileWatcher()
{
}

void VaultFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl furl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    emit fileDeleted(furl);
}

void VaultFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl furl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    emit fileAttributeChanged(furl);
}

void VaultFileWatcher::onFileRename(const QUrl &fromUrl, const QUrl &toUrl)
{
    QUrl furl = VaultHelper::instance()->pathToVaultVirtualUrl(fromUrl.path());
    QUrl turl = VaultHelper::instance()->pathToVaultVirtualUrl(toUrl.path());
    emit fileRename(furl, turl);
}

void VaultFileWatcher::onSubfileCreated(const QUrl &url)
{
    QUrl vurl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    if (vurl.toString().endsWith(QString(QDir::separator()) + ".hidden")) {
        emit fileRename(QUrl(), vurl);
    } else {
        emit subfileCreated(vurl);
    }
}
