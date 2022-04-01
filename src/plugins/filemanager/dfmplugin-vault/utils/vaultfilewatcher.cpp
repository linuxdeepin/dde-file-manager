/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "vaultfilewatcher.h"
#include "dfm-base/base/urlroute.h"
#include "utils/vaulthelper.h"
#include "private/vaultfilewatcherprivate.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultFileWatcher::VaultFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new VaultFileWatcherPrivate(url, this), parent)
{
    dptr = dynamic_cast<VaultFileWatcherPrivate *>(d.data());
    QString path = url.path().contains(VaultHelper::instance()->rootUrl().path()) ? url.path() : UrlRoute::urlToPath(url);
    QUrl localUrl = QUrl::fromLocalFile(path);
    dptr->proxyStaging = WatcherFactory::create<AbstractFileWatcher>(localUrl);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &VaultFileWatcher::onFileAttributeChanged);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileDeleted,
            this, &VaultFileWatcher::onFileDeleted);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileRename,
            this, &VaultFileWatcher::onFileRename);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::subfileCreated,
            this, &VaultFileWatcher::onSubfileCreated);
}

VaultFileWatcher::~VaultFileWatcher()
{
}

void VaultFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl vaultUrl = url;
    vaultUrl.setScheme(VaultHelper::instance()->scheme());
    vaultUrl.setHost("");
    emit fileDeleted(vaultUrl);
}

void VaultFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl vaultUrl = url;
    vaultUrl.setScheme(VaultHelper::instance()->scheme());
    vaultUrl.setHost("");
    emit fileAttributeChanged(vaultUrl);
}

void VaultFileWatcher::onFileRename(const QUrl &fromUrl, const QUrl &toUrl)
{
    QUrl vaultFromUrl = fromUrl;
    vaultFromUrl.setScheme(VaultHelper::instance()->scheme());
    vaultFromUrl.setHost("");
    QUrl vaultToUrl = toUrl;
    vaultToUrl.setScheme(VaultHelper::instance()->scheme());
    vaultToUrl.setHost("");
    emit fileRename(vaultFromUrl, vaultToUrl);
}

void VaultFileWatcher::onSubfileCreated(const QUrl &url)
{
    QUrl vaultUrl = url;
    vaultUrl.setScheme(VaultHelper::instance()->scheme());
    vaultUrl.setHost("");
    emit subfileCreated(vaultUrl);
}
