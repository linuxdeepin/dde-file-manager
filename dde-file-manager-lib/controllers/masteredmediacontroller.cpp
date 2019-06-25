#include "masteredmediacontroller.h"
#include "dfmevent.h"
#include "ddiskmanager.h"
#include "ddiskdevice.h"
#include "dblockdevice.h"
#include "deviceinfo/udisklistener.h"
#include "app/define.h"
#include "models/desktopfileinfo.h"
#include "models/masteredmediafileinfo.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/burnoptdialog.h"
#include "interfaces/dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"
#include "disomaster.h"
#include "shutil/fileutils.h"

#include <QRegularExpression>
#include <QStandardPaths>
#include <QProcess>

class DFMShadowedDirIterator : public DDirIterator
{
public:
    DFMShadowedDirIterator(const QUrl &path,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags)
    {
        QRegularExpression re("^(.*?)/(disk_files|staging_files)(.*)$");
        QString device(path.path());
        auto rem = re.match(device);
        if (rem.hasMatch()) {
            QString udiskspath = rem.captured(1);
            udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
            QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
            QSharedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
            if (blkdev->mountPoints().size()) {
                DUrl mnturl = DUrl(QString(blkdev->mountPoints().front()));
                mnturl.setScheme(FILE_SCHEME);
                mntpoint = mnturl.toLocalFile();
            }
            while (*mntpoint.rbegin() == '/') {
                mntpoint.chop(1);
            }
            devfile = rem.captured(1);
            if (diskdev->opticalBlank()) {
                //blank disc
                iterator.clear();
                stagingiterator = QSharedPointer<QDirIterator>(
                                      new QDirIterator(MasteredMediaController::getStagingFolder(DUrl(path)).path(),
                                                       nameFilters, filter, flags)
                                  );
                return;
            }
            QString realpath = mntpoint + rem.captured(3);
            iterator = QSharedPointer<QDirIterator>(new QDirIterator(realpath, nameFilters, filter, flags));
            stagingiterator = QSharedPointer<QDirIterator>(
                                  new QDirIterator(MasteredMediaController::getStagingFolder(DUrl(path)).path(),
                                                   nameFilters, filter, flags)
                              );
        }
    }

    DUrl next() override
    {
        return changeSchemeUpdate(DUrl::fromLocalFile(iterator && iterator->hasNext() ? iterator->next() : (iterator = QSharedPointer<QDirIterator>(Q_NULLPTR), stagingiterator->next())));
    }

    bool hasNext() const override
    {
        return (iterator && iterator->hasNext()) ||
               (stagingiterator && stagingiterator->hasNext());
    }

    QString fileName() const override
    {
        return iterator ? iterator->fileName() : stagingiterator->fileName();
    }

    DUrl fileUrl() const override
    {
        return changeScheme(DUrl::fromLocalFile(iterator ? iterator->filePath() : stagingiterator->filePath()));
    }

    const DAbstractFileInfoPointer fileInfo() const override
    {
        DUrl url = fileUrl();

        return DAbstractFileInfoPointer(new MasteredMediaFileInfo(url));
    }

    DUrl url() const override
    {
        return changeScheme(DUrl::fromLocalFile(iterator ? iterator->path() : stagingiterator->path()));
    }

private:
    QSharedPointer<QDirIterator> iterator;
    QSharedPointer<QDirIterator> stagingiterator;
    QString mntpoint;
    QString devfile;
    QSet<QString> seen;
    QSet<DUrl> skip;
    DUrl changeScheme(DUrl in) const
    {
        DUrl burntmp = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/diskburn/");
        QString stagingroot = burntmp.path() + QString(devfile).replace('/','_');
        DUrl ret;
        ret.setScheme(BURN_SCHEME);
        QString path = in.path();
        if (burntmp.isParentOf(in)) {
            path.replace(stagingroot, devfile + "/staging_files");
        } else {
            path.replace(mntpoint, devfile + "/disk_files");
        }
        ret.setPath(path);
        if (skip.contains(ret)) {
            return DUrl();
        }
        return ret;
    }
    DUrl changeSchemeUpdate(DUrl in)
    {
        DUrl ret = changeScheme(in);
        QRegularExpression re("^(.*?)/(disk_files|staging_files)(.*)$");
        auto rem = re.match(ret.path());
        Q_ASSERT(rem.hasMatch());
        if (seen.contains(rem.captured(3))) {
            skip.insert(ret);
            return DUrl();
        }
        seen.insert(rem.captured(3));
        return ret;
    }
};

class MasteredMediaFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    MasteredMediaFileWatcherPrivate(MasteredMediaFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() Q_DECL_OVERRIDE;
    bool stop() Q_DECL_OVERRIDE;

    QPointer<DAbstractFileWatcher> proxyStaging;
    QPointer<DAbstractFileWatcher> proxyOnDisk;
    QScopedPointer<DDiskManager> diskm;

    Q_DECLARE_PUBLIC(MasteredMediaFileWatcher)
};

bool MasteredMediaFileWatcherPrivate::start()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->startWatcher();
}

bool MasteredMediaFileWatcherPrivate::stop()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->stopWatcher();
}

MasteredMediaFileWatcher::MasteredMediaFileWatcher(const DUrl &url, QObject *parent)
    : DAbstractFileWatcher(*new MasteredMediaFileWatcherPrivate(this), url, parent)
{
    Q_D(MasteredMediaFileWatcher);

    DUrl url_staging = MasteredMediaController::getStagingFolder(url);
    d->proxyStaging = QPointer<DAbstractFileWatcher>(new DFileProxyWatcher(url_staging,
                                 new DFileWatcher(url_staging.path()),
                                 [](const DUrl& in)->DUrl {
                                     QString tmpp = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/diskburn/";
                                     QString relp = in.path().mid(in.path().indexOf(tmpp) + tmpp.length());
                                     QString devp = relp.left(relp.indexOf('/'));
                                     QString imgp = relp.mid(relp.indexOf('/') + 1);
                                     devp.replace('_', '/');
                                     DUrl ret = DUrl(devp + "/staging_files/" + imgp);
                                     ret.setScheme(BURN_SCHEME);
                                     return ret;
                                 }
    ));
    d->proxyStaging->moveToThread(thread());
    d->proxyStaging->setParent(this);

    connect(d->proxyStaging, &DAbstractFileWatcher::fileAttributeChanged, this, &MasteredMediaFileWatcher::onFileAttributeChanged);
    connect(d->proxyStaging, &DAbstractFileWatcher::fileDeleted, this, &MasteredMediaFileWatcher::onFileDeleted);
    connect(d->proxyStaging, &DAbstractFileWatcher::fileMoved, this, &MasteredMediaFileWatcher::onFileMoved);
    connect(d->proxyStaging, &DAbstractFileWatcher::subfileCreated, this, &MasteredMediaFileWatcher::onSubfileCreated);

    d->proxyOnDisk.clear();

    QRegularExpression re("^(.*?)/(disk_files|staging_files)(.*)$");
    QString device(url.path());
    auto rem = re.match(device);
    if (!rem.hasMatch()) {
        return;
    }
    QString udiskspath = rem.captured(1);
    udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));

    if (blkdev->mountPoints().size()) {
        DUrl url_mountpoint = DUrl::fromLocalFile(blkdev->mountPoints().front());
        d->proxyOnDisk = QPointer<DAbstractFileWatcher>(new DFileWatcher(url_mountpoint.path()));
        d->proxyOnDisk->moveToThread(thread());
        d->proxyOnDisk->setParent(this);
        connect(d->proxyOnDisk, &DAbstractFileWatcher::fileDeleted, this, [this, url] {emit fileDeleted(url);});
    } else { //The disc is not mounted, i.e. the disc is blank
        d->diskm.reset(new DDiskManager(this));
        connect(d->diskm.data(), &DDiskManager::opticalChanged, this,
            [this, blkdev, url](const QString &path) {
                if (path == blkdev->drive()) {
                    emit fileDeleted(url);
                }
            });
        d->diskm->setWatchChanges(true);
    }

}

void MasteredMediaFileWatcher::onFileDeleted(const DUrl &url)
{
    emit fileDeleted(url);
}

void MasteredMediaFileWatcher::onFileAttributeChanged(const DUrl &url)
{
    emit fileAttributeChanged(url);
}

void MasteredMediaFileWatcher::onFileMoved(const DUrl &fromUrl, const DUrl &toUrl)
{
    emit fileMoved(fromUrl, toUrl);
}

void MasteredMediaFileWatcher::onSubfileCreated(const DUrl &url)
{
    emit subfileCreated(url);
}

MasteredMediaController::MasteredMediaController(QObject *parent) : DAbstractFileController(parent)
{

}

bool MasteredMediaController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->openFile(event->sender(), url);
}

bool MasteredMediaController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->openFileByApp(event->sender(), event->appName(), url);
}

bool MasteredMediaController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.path().indexOf("/disk_files/") != -1) {
            DUrl local_file = DUrl::fromLocalFile(MasteredMediaFileInfo(i).extraProperties()["mm_backer"].toString());
            lst.push_back(local_file);
        }
    }

    return fileService->compressFiles(event->sender(), lst);
}

bool MasteredMediaController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.path().indexOf("/disk_files/") != -1) {
            DUrl local_file = DUrl::fromLocalFile(MasteredMediaFileInfo(i).extraProperties()["mm_backer"].toString());
            lst.push_back(local_file);
        }
    }

    return fileService->decompressFile(event->sender(), lst);
}

bool MasteredMediaController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.path().indexOf("/staging_files/") != -1) {
            lst.push_back(getStagingFolder(i));
        }
    }

    return fileService->deleteFiles(event->sender(), lst, false);
}

DUrlList MasteredMediaController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList lst, retlst;
    for (auto &i : event->urlList()) {
        if (i.path().indexOf("/staging_files/") != -1) {
            lst.push_back(getStagingFolder(i));
            retlst.push_back(i);
        }
    }
    fileService->deleteFiles(event->sender(), lst, false);

    return retlst;
}

bool MasteredMediaController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    //TODO: hide/disable the menu item if no selected file is already on disc.
    DUrlList lst;
    for (auto &i : event->urlList()) {
        DAbstractFileInfoPointer fp = fileService->createFileInfo(event->sender(), i);
        if (!DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).isParentOf(
             DUrl::fromLocalFile(fp->extraProperties()["mm_backer"].toString()))) {
            lst.push_back(DUrl::fromLocalFile(fp->extraProperties()["mm_backer"].toString()));
        }
    }
    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(lst), event->action());

    return !lst.empty();
}

DUrlList MasteredMediaController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    DUrlList src = event->urlList();
    DUrl dst = event->targetUrl();

    if (src.size() == 1) {
        QRegularExpression re("^(.*?)/(disk_files|staging_files)(.*)$");
        auto rem = re.match(dst.path());
        Q_ASSERT(rem.hasMatch());
        QString dev(rem.captured(1));
        bool is_blank = ISOMaster->getDevicePropertyCached(dev).formatted;
        QString dstdirpath = getStagingFolder(DUrl(dev + "/staging_files" + rem.captured(3))).path();
        QDir dstdir = QDir(dstdirpath);
        DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), src.front());
        QSet<QString> image_types = {"application/x-cd-image", "application/x-iso9660-image"};
        if (is_blank && image_types.contains(fi->mimeTypeName()) && dstdir.count() == 0) {
            int r = DThreadUtil::runInMainThread(dialogManager, &DialogManager::showOpticalImageOpSelectionDialog, DFMUrlBaseEvent(event->sender(), dst));
            if (r == 1) {
                DThreadUtil::runInMainThread([=] {
                    QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(dev));
                    bd->setISOImage(src.front());
                    bd->exec();
                });
                return DUrlList{};
            }
            if (r == 0) {
                return DUrlList{};
            }
        }
    }

    DUrl tmpdst = getStagingFolder(dst);
    mkpath(tmpdst);

    return fileService->pasteFile(event->sender(), event->action(), tmpdst, src);
}

const DAbstractFileInfoPointer MasteredMediaController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new MasteredMediaFileInfo(event->url()));
}

const DDirIteratorPointer MasteredMediaController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new DFMShadowedDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

bool MasteredMediaController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
{
    event->ignore();

    return false;
}

bool MasteredMediaController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    event->ignore();

    return false;
}

bool MasteredMediaController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    if (event->url().path().indexOf("/disk_files/") == -1) {
        return false;
    }

    const QString &current_dir = QDir::currentPath();

    QString backer = MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString();
    if (!backer.length()) {
        return false;
    }
    QDir::setCurrent(backer);

    bool ok = QProcess::startDetached(FileUtils::defaultTerminalPath());

    QDir::setCurrent(current_dir);

    return ok;
}

bool MasteredMediaController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    if (event->fileUrl().path().indexOf("/disk_files/") == -1) {
        return false;
    }

    if (event->toUrl().scheme() == BURN_SCHEME) {
        return false;
    }

    DUrl local_url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->fileUrl()).extraProperties()["mm_backer"].toString());
    return fileService->createSymlink(event->sender(), local_url, event->toUrl());
}

bool MasteredMediaController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    event->ignore();
    return false;
}

bool MasteredMediaController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent>& event) const
{
    event->ignore();
    return false;
}

QList<QString> MasteredMediaController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    event->ignore();
    return {QString()};
}

DAbstractFileWatcher *MasteredMediaController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new MasteredMediaFileWatcher(event->url());
}

void MasteredMediaController::mkpath(DUrl path) const
{
    if (path.parentUrl() == path) {
        return;
    }
    if (fileService->createFileInfo(this, path)->isDir()) {
        return;
    }
    if (fileService->mkdir(this, path)) {
        return;
    }
    mkpath(path.parentUrl());
    fileService->mkdir(this, path);
}

DUrl MasteredMediaController::getStagingFolder(DUrl dst)
{
    QRegularExpression re("^(.*?)/(disk_files|staging_files)(.*)$");
    auto rem = re.match(dst.path());
    if (!rem.hasMatch()) {
        return DUrl();
    }
    return DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                  + "/diskburn/" + rem.captured(1).replace('/','_')
                  + rem.captured(3));
}
