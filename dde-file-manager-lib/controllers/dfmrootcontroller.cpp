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

    QString udiskspath = "/org/freedesktop/UDisks2/block_devices" + fi->fileUrl().path().chopped(QString(".localdisk").length());
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
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + d + ".userdir")));
        if (fp->exists()) {
            ret.push_back(fp);
        }
    }

    QSet<QString> udisksuuids;
    DDiskManager dummy;
    for (auto blks : dummy.blockDevices()) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        if (blk->idUUID().length() == 0) {
            //extended partition doesn't have a UUID.
            continue;
        }
        udisksuuids.insert(blk->idUUID());
        if (blk->hintIgnore()) {
            continue;
        }
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + QString(blk->device()).mid(QString("/dev/").length()) + ".localdisk")));
        ret.push_back(fp);
    }

    for (auto gvfsmp : DGioVolumeManager::getMounts()) {
        if (gvfsmp->getVolume() && udisksuuids.contains(gvfsmp->getVolume()->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_UUID))) {
            continue;
        }
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + QUrl::toPercentEncoding(gvfsmp->getRootFile()->path()) + ".gvfsmp")));
        ret.push_back(fp);
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
        Q_EMIT wpar->subfileCreated(DUrl(DFMROOT_ROOT + QUrl::toPercentEncoding(mnt->getRootFile()->path()) + ".gvfsmp"));
    }));
    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::mountRemoved, [wpar](QExplicitlySharedDataPointer<DGioMount> mnt) {
        Q_EMIT wpar->fileDeleted(DUrl(DFMROOT_ROOT + QUrl::toPercentEncoding(mnt->getRootFile()->path()) + ".gvfsmp"));
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::fileSystemAdded, [wpar](const QString &blks) {
        Q_EMIT wpar->subfileCreated(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + ".localdisk"));
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::fileSystemRemoved, [wpar](const QString &blks) {
        Q_EMIT wpar->fileDeleted(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + ".localdisk"));
    }));

    for (auto devs : udisksmgr->blockDevices()) {
        blkdevs.push_back(QSharedPointer<DBlockDevice>(DDiskManager::createBlockDevice(devs)));
        blkdevs.back()->setWatchChanges(true);
        connections.push_back(QObject::connect(blkdevs.back().data(), &DBlockDevice::idLabelChanged, [wpar, devs](const QString &) {
            Q_EMIT wpar->fileAttributeChanged(DUrl(DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + ".localdisk"));
        }));
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

    started = false;

    return true;
}
