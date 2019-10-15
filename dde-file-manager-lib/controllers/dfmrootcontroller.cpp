/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#include "dfmrootcontroller.h"
#include "dfmevent.h"
#include "models/dfmrootfileinfo.h"
#include "private/dabstractfilewatcher_p.h"

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiovolumemanager.h>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

class DFMRootFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    DFMRootFileWatcherPrivate(DFMRootFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;

private:
    QSharedPointer<DGioVolumeManager> vfsmgr;
    QSharedPointer<DDiskManager> udisksmgr;
    QList<QMetaObject::Connection> connections;
    QList<QSharedPointer<DBlockDevice>> blkdevs;

    Q_DECLARE_PUBLIC(DFMRootFileWatcher)
};

DFMRootController::DFMRootController(QObject *parent) : DAbstractFileController(parent)
{

}

bool DFMRootController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    DAbstractFileInfoPointer fi(new DFMRootFileInfo(event->fromUrl()));
    if (!fi->canRename()) {
        return false;
    }

    QString udiskspath = "/org/freedesktop/UDisks2/block_devices" + fi->fileUrl().path().chopped(QString("." SUFFIX_UDISKS).length());
    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
    Q_ASSERT(blk && blk->path().length() > 0);

    blk->setLabel(event->toUrl().path(), {});
}

const QList<DAbstractFileInfoPointer> DFMRootController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> ret;
    if (event->url().scheme() != DFMROOT_SCHEME || event->url().path() != "/") {
        return ret;
    }

    static const QList<QString> udir = {"desktop", "videos", "music", "pictures", "documents", "downloads"};
    for (auto d : udir) {
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + d + "." SUFFIX_USRDIR)));
        if (fp->exists()) {
            ret.push_back(fp);
        }
    }

    DDiskManager dummy;
    for (auto blks : dummy.blockDevices()) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (!blk->hasFileSystem() && !drv->mediaCompatibility().join(" ").contains("optical") && !blk->isEncrypted()) {
            continue;
        }
        if ((blk->hintIgnore() && !blk->isEncrypted()) || blk->cryptoBackingDevice().length() > 1) {
            continue;
        }
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + QString(blk->device()).mid(QString("/dev/").length()) + "." SUFFIX_UDISKS)));
        ret.push_back(fp);
    }

    for (auto gvfsmp : DGioVolumeManager::getMounts()) {
        if (gvfsmp->getVolume() && gvfsmp->getVolume()->volumeMonitorName().endsWith("UDisks2")) {
            continue;
        }
        if (DUrl(gvfsmp->getRootFile()->uri()).scheme() == BURN_SCHEME) {
            continue;
        }
        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        url.setPath("/" + QUrl::toPercentEncoding(gvfsmp->getRootFile()->path()) + "." SUFFIX_GVFSMP);
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(url));
        if (fp->exists()) {
            ret.push_back(fp);
        }
    }

    return ret;
}

const DAbstractFileInfoPointer DFMRootController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new DFMRootFileInfo(event->url()));
}

DAbstractFileWatcher *DFMRootController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFMRootFileWatcher(event->url());
}

DFMRootFileWatcher::DFMRootFileWatcher(const DUrl &url, QObject *parent):
    DAbstractFileWatcher(*new DFMRootFileWatcherPrivate(this), url, parent)
{

}

bool DFMRootFileWatcherPrivate::start()
{
    Q_Q(DFMRootFileWatcher);

    if (q->fileUrl().path() != "/" || started) {
        return false;
    }

    if (!vfsmgr) {
        vfsmgr.reset(new DGioVolumeManager);
    }
    if (!udisksmgr) {
        udisksmgr.reset(new DDiskManager);
    }

    udisksmgr->setWatchChanges(true);

    DFMRootFileWatcher *wpar = qobject_cast<DFMRootFileWatcher*>(q);

    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::mountAdded, [wpar](QExplicitlySharedDataPointer<DGioMount> mnt) {
        if (mnt->getVolume() && mnt->getVolume()->volumeMonitorName().endsWith("UDisks2")) {
            return;
        }
        if (DUrl(mnt->getRootFile()->uri()).scheme() == BURN_SCHEME) {
            return;
        }
        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        url.setPath("/" + QUrl::toPercentEncoding(mnt->getRootFile()->path()) + "." SUFFIX_GVFSMP);
        Q_EMIT wpar->subfileCreated(url);
    }));
    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::mountRemoved, [wpar](QExplicitlySharedDataPointer<DGioMount> mnt) {
        if (mnt->getVolume() && mnt->getVolume()->volumeMonitorName().endsWith("UDisks2")) {
            return;
        }
        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        url.setPath("/" + QUrl::toPercentEncoding(mnt->getRootFile()->path()) + "." SUFFIX_GVFSMP);
        Q_EMIT wpar->fileDeleted(url);
    }));
    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::volumeAdded, [](QExplicitlySharedDataPointer<DGioVolume> vol) {
        if (vol->volumeMonitorName().contains(QRegularExpression("(MTP|GPhoto2|Afc)$"))) {
            vol->mount();
        }
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::blockDeviceAdded, [wpar, this](const QString &blks) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

        if (!blk->hasFileSystem() && !drv->mediaCompatibility().join(" ").contains("optical") && !blk->isEncrypted()) {
            return;
        }
        if ((blk->hintIgnore() && !blk->isEncrypted()) || blk->cryptoBackingDevice().length() > 1) {
            return;
        }

        blkdevs.push_back(blk);
        blk->setWatchChanges(true);

        if (blk->isEncrypted()) {
            QSharedPointer<DBlockDevice> ctblk(DDiskManager::createBlockDevice(blk->cleartextDevice()));
            blkdevs.push_back(ctblk);
            ctblk->setWatchChanges(true);
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::cleartextDeviceChanged, [wpar, blks](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::idLabelChanged, [wpar, blks](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::mountPointsChanged, [wpar, blks](const QByteArrayList &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
        } else {
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::idLabelChanged, [wpar, blks](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::mountPointsChanged, [wpar, blks](const QByteArrayList &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
        }

        Q_EMIT wpar->subfileCreated(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::blockDeviceRemoved, [wpar](const QString &blks) {
        Q_EMIT wpar->fileDeleted(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
    }));

    for (auto devs : udisksmgr->blockDevices()) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(devs));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

        if (!blk->hasFileSystem() && !drv->mediaCompatibility().join(" ").contains("optical") && !blk->isEncrypted()) {
            continue;
        }
        if ((blk->hintIgnore() && !blk->isEncrypted()) || blk->cryptoBackingDevice().length() > 1) {
            continue;
        }

        blkdevs.push_back(blk);
        blk->setWatchChanges(true);

        if (blk->isEncrypted()) {
            QSharedPointer<DBlockDevice> ctblk(DDiskManager::createBlockDevice(blk->cleartextDevice()));
            blkdevs.push_back(ctblk);
            ctblk->setWatchChanges(true);
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::cleartextDeviceChanged, [wpar, devs](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::idLabelChanged, [wpar, devs](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::mountPointsChanged, [wpar, devs](const QByteArrayList &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
        } else {
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::idLabelChanged, [wpar, devs](const QString &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
            connections.push_back(QObject::connect(blk.data(), &DBlockDevice::mountPointsChanged, [wpar, devs](const QByteArrayList &) {
                Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
            }));
        }
    }

    started = true;
    return true;
}

bool DFMRootFileWatcherPrivate::stop()
{
    if (!started) {
        return false;
    }

    udisksmgr->setWatchChanges(false);

    for(auto &conn : connections) {
        QObject::disconnect(conn);
    }
    connections.clear();

    blkdevs.clear();

    vfsmgr.clear();
    udisksmgr.clear();

    started = false;

    return true;
}
