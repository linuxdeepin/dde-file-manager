// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masteredmediafilewatcher.h"
#include "masteredmediafilewatcher_p.h"
#include "utils/opticalhelper.h"
#include "utils/opticalsignalmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/universalutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

using namespace GlobalServerDefines;

MasteredMediaFileWatcherPrivate::MasteredMediaFileWatcherPrivate(const QUrl &fileUrl, MasteredMediaFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool MasteredMediaFileWatcherPrivate::start()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->startWatcher();
}

bool MasteredMediaFileWatcherPrivate::stop()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->stopWatcher();
}

MasteredMediaFileWatcher::MasteredMediaFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new MasteredMediaFileWatcherPrivate(url, this), parent)
{
    dptr = dynamic_cast<MasteredMediaFileWatcherPrivate *>(d.data());
    QString dev { OpticalHelper::burnDestDevice(url) };
    if (dev.isEmpty())
        return;
    OpticalHelper::createStagingFolder(dev);

    QUrl urlStaging { OpticalHelper::localStagingFile(url) };
    if (!urlStaging.isValid() || urlStaging.isEmpty())
        return;

    dptr->proxyStaging = WatcherFactory::create<AbstractFileWatcher>(urlStaging, false);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &MasteredMediaFileWatcher::onFileAttributeChanged);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileDeleted,
            this, &MasteredMediaFileWatcher::onFileDeleted);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileRename,
            this, &MasteredMediaFileWatcher::onFileRename);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::subfileCreated,
            this, &MasteredMediaFileWatcher::onSubfileCreated);

    dptr->proxyOnDisk.clear();
    QString devFile { OpticalHelper::burnDestDevice(url) };
    QString id { DeviceUtils::getBlockDeviceId(devFile) };
    auto &&map = DevProxyMng->queryBlockInfo(id);
    dptr->curMnt = qvariant_cast<QString>(map[DeviceProperty::kMountPoint]);
    QUrl mntUrl { QUrl::fromLocalFile(dptr->curMnt) };
    dptr->proxyOnDisk = WatcherFactory::create<AbstractFileWatcher>(mntUrl, false);
    connect(dptr->proxyOnDisk.data(), &AbstractFileWatcher::fileDeleted, this, [this, mntUrl, id](const QUrl &url) {
        if (UniversalUtils::urlEquals(mntUrl, url))
            onMountPointDeleted(id);
    });
    // pull out empty disc!
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, [this, map](const QString &id) {
        bool isBlank { map[DeviceProperty::kOpticalBlank].toBool() };
        if (isBlank)
            onMountPointDeleted(id);
    });
}

void MasteredMediaFileWatcher::onFileDeleted(const QUrl &url)
{
    if (!UniversalUtils::urlEquals(url, dptr->proxyStaging->url())) {
        QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
        emit fileDeleted(realUrl);
    }
}

void MasteredMediaFileWatcher::onMountPointDeleted(const QString &id)
{
    const auto &discUrl { OpticalHelper::transDiscRootById(id) };
    if (discUrl.isValid()) {
        emit OpticalSignalManager::instance()->discUnmounted(discUrl);
        emit fileDeleted(discUrl);
    }
}

void MasteredMediaFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    if (!UniversalUtils::urlEquals(url, dptr->proxyStaging->url())) {
        QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
        emit fileAttributeChanged(realUrl);
    }
}

void MasteredMediaFileWatcher::onFileRename(const QUrl &fromUrl, const QUrl &toUrl)
{
    auto &&stagingUrl { dptr->proxyStaging->url() };
    if (!UniversalUtils::urlEquals(fromUrl, stagingUrl) || !UniversalUtils::urlEquals(toUrl, stagingUrl)) {
        QUrl realFromUrl { OpticalHelper::tansToBurnFile(fromUrl) };
        QUrl realToUrl { OpticalHelper::tansToBurnFile(toUrl) };
        emit fileRename(realFromUrl, realToUrl);
    }
}

void MasteredMediaFileWatcher::onSubfileCreated(const QUrl &url)
{
    if (!UniversalUtils::urlEquals(url, dptr->proxyStaging->url())) {
        QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
        if (OpticalHelper::isDupFileNameInPath(dptr->curMnt, realUrl)) {
            qWarning() << "Dup file: " << url;
            return;
        }
        emit subfileCreated(realUrl);
    }
}
