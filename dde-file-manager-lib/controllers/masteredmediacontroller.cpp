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
#include "disomaster.h"

#include <QRegularExpression>
#include <QStandardPaths>

class DFMShadowedDirIterator : public DDirIterator
{
public:
    DFMShadowedDirIterator(const QUrl &path,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags)
    {
        QRegularExpression re("^(.*)/disk_files/(.*)$");
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
            if (*mntpoint.rbegin() != '/') {
                mntpoint += '/';
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
            QString realpath = mntpoint + rem.captured(2);
            iterator = QSharedPointer<QDirIterator>(new QDirIterator(realpath, nameFilters, filter, flags));
            stagingiterator = QSharedPointer<QDirIterator>(
                                  new QDirIterator(MasteredMediaController::getStagingFolder(DUrl(path)).path(),
                                                   nameFilters, filter, flags)
                              );
        }
    }

    DUrl next() override
    {
        return changeScheme(DUrl::fromLocalFile(iterator && iterator->hasNext() ? iterator->next() : (iterator = QSharedPointer<QDirIterator>(Q_NULLPTR), stagingiterator->next())));
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
        /*const QFileInfo &info = iterator->fileInfo();

        if (info.suffix() == DESKTOP_SURRIX) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(info));
        }*/
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
    DUrl changeScheme(DUrl in) const
    {
        DUrl burntmp = DUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/diskburn/");
        burntmp.setScheme(FILE_SCHEME);
        QString stagingroot = burntmp.path() + QString(devfile).replace('/','_') + "/";
        DUrl ret;
        ret.setScheme(BURN_SCHEME);
        QString path = in.path();
        if (burntmp.isParentOf(in)) {
            path.replace(stagingroot, devfile + "/staging_files/");
        } else {
            path.replace(mntpoint, devfile + "/disk_files/");
        }
        ret.setPath(path);
        return ret;
    }
};

MasteredMediaController::MasteredMediaController(QObject *parent) : DAbstractFileController(parent)
{

}

bool MasteredMediaController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).toQFileInfo().absoluteFilePath());

    return fileService->openFile(event->sender(), url);
}

bool MasteredMediaController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).toQFileInfo().absoluteFilePath());

    return fileService->openFileByApp(event->sender(), event->appName(), url);
}

bool MasteredMediaController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        lst.push_back(getStagingFolder(i));
    }

    return fileService->deleteFiles(event->sender(), lst, false);
}

DUrlList MasteredMediaController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    DUrlList src = event->urlList();
    DUrl dst = event->targetUrl();

    if (src.size() == 1) {
        QRegularExpression re("^(.*)/disk_files/(.*)$");
        auto rem = re.match(dst.path());
        Q_ASSERT(rem.hasMatch());
        QString dev(rem.captured(1));
        bool is_blank = ISOMaster->getDevicePropertyCached(dev).formatted;
        QString dstdirpath = getStagingFolder(DUrl(dev + "/staging_files/")).path();
        QDir dstdir = QDir(dstdirpath);
        DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), src.front());
        if (is_blank && fi->mimeTypeName() == "application/x-cd-image" && dstdir.count() == 0) {
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

const DAbstractFileInfoPointer MasteredMediaController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new MasteredMediaFileInfo(event->url()));
}

const DDirIteratorPointer MasteredMediaController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new DFMShadowedDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

bool MasteredMediaController::shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const
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
    event->ignore();

    return false;
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
    DUrl stagingUrl = event->fileUrl();
    stagingUrl.setPath(stagingUrl.path().replace("disk_files", "staging_files"));

    DAbstractFileInfoPointer ifo = fileService->createFileInfo(this, stagingUrl);
    if (!ifo || !ifo->parentUrl().isValid() ) {
        return nullptr;
    }
    return new DFileProxyWatcher(DUrl::fromLocalFile(ifo->toQFileInfo().absoluteFilePath()),
                                 new DFileWatcher(ifo->toQFileInfo().absoluteFilePath()),
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
    );
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
    QRegularExpression re("^(.*)/(disk_files|staging_files)/(.*)$");
    auto rem = re.match(dst.path());
    if (!rem.hasMatch()) {
        return DUrl();
    }
    DUrl tmpdst = DUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                  + "/diskburn/" + rem.captured(1).replace('/','_') + "/"
                  + rem.captured(3));
    tmpdst.setScheme(FILE_SCHEME);
    return tmpdst;
}
