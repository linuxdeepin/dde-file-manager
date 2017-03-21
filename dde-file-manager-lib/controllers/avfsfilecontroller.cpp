#include "avfsfilecontroller.h"
#include "interfaces/dfileinfo.h"
#include "dfmstandardpaths.h"
#include <QStandardPaths>
#include <QDir>
#include "models/avfsfileinfo.h"
#include "dfileinfo.h"
#include <QDirIterator>
#include <dfilewatcher.h>
#include "dfileservices.h"
#include "shutil/fileutils.h"
#include "dfmevent.h"

class AVFSIterator : public DDirIterator
{
public:
    AVFSIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;

private:
    QDirIterator *iterator;
    DUrl currentUrl;
};

AVFSIterator::AVFSIterator(const DUrl &url, const QStringList &nameFilters, QDir::Filters filter, QDirIterator::IteratorFlags flags):
    DDirIterator()
{
    QString realPath = AVFSFileInfo::realDirUrl(url).toLocalFile();
    iterator = new QDirIterator(realPath, nameFilters, filter, flags);
    currentUrl = url;
}

DUrl AVFSIterator::next()
{
    QString realPath = iterator->next();
    Q_UNUSED(realPath);
    DUrl url = DUrl::fromAVFSFile(currentUrl.path() + "/" + fileName());
    return url;
}

bool AVFSIterator::hasNext() const
{
    return iterator->hasNext();
}

QString AVFSIterator::fileName() const
{
    return fileInfo()->fileName();
}

QString AVFSIterator::filePath() const
{
    return fileInfo()->filePath();
}

const DAbstractFileInfoPointer AVFSIterator::fileInfo() const
{
    DUrl url = DUrl::fromAVFSFile(currentUrl.path() + "/" + iterator->fileName());
    return DAbstractFileInfoPointer(new AVFSFileInfo(url));
}

QString AVFSIterator::path() const
{
    return fileInfo()->filePath();
}

AVFSFileController::AVFSFileController(QObject *parent):
    DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer AVFSFileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    DAbstractFileInfoPointer info(new AVFSFileInfo(event->fileUrl()));

    return info;
}

const DDirIteratorPointer AVFSFileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new AVFSIterator(event->fileUrl(), event->nameFilters(), event->filters(), event->flags()));
}

DAbstractFileWatcher *AVFSFileController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    QString realPath = AVFSFileInfo::realDirUrl(event->fileUrl()).toLocalFile();

    return new DFileWatcher(realPath);
}

bool AVFSFileController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DFileService::instance()->openFileLocation(realUrl(event->url()), event->sender());
}

bool AVFSFileController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(realUrl(event->url()), event->sender());
}

bool AVFSFileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->appName(), realUrl(event->url()), event->sender());
}

bool AVFSFileController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DUrlList realUrlList;

    foreach (const DUrl& url, event->urlList()) {
        realUrlList << realUrl(url);
    }

    return DFileService::instance()->writeFilesToClipboard(event->action(), realUrlList, event->sender());
}

bool AVFSFileController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    return DFileService::instance()->openInTerminal(realUrl(event->fileUrl()), event->sender());
}

DUrl AVFSFileController::realUrl(const DUrl &url)
{
    return AVFSFileInfo::realFileUrl(url);
}

QString AVFSFileController::findArchFileRootPath(const DUrl &url)
{
    QStringList items = url.path().split("/");
    for(int i = 0; i< items.size(); i++){
        QString path = "";
        for(int j = 0; j<=i; j++){
            path += (items.at(j) + "/");
        }
        if(path.endsWith("/"))
            path.chop(1);
        if(FileUtils::isArchive(path))
            return path;
    }
    return QString();
}

