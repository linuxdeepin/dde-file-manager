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
    QString realPath = AVFSFileInfo::realFileUrl(AVFSFileController::findArchFileRootPath(url), url).toLocalFile();
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
    QString archRootFilePath = AVFSFileController::findArchFileRootPath(url);
    return DAbstractFileInfoPointer(new AVFSFileInfo(archRootFilePath, url));
}

QString AVFSIterator::path() const
{

}

AVFSFileController::AVFSFileController(QObject *parent):
    DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer AVFSFileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    QString archRootFilePath = findArchFileRootPath(fileUrl);

    DAbstractFileInfoPointer info(new AVFSFileInfo(archRootFilePath, fileUrl));

    return info;
}

const DDirIteratorPointer AVFSFileController::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags, bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new AVFSIterator(fileUrl, nameFilters, filters, flags));
}

DAbstractFileWatcher *AVFSFileController::createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const
{
    accepted = true;

    QString realPath = AVFSFileInfo::realFileUrl(findArchFileRootPath(fileUrl), fileUrl).toLocalFile();

    return new DFileWatcher(realPath, parent);
}

bool AVFSFileController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    return DFileService::instance()->openFileLocation(realUrl(fileUrl));
}

bool AVFSFileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    return DFileService::instance()->openFile(realUrl(fileUrl));
}

bool AVFSFileController::openFileByApp(const DUrl &fileUrl, const QString &app, bool &accepted) const
{
    accepted = true;
    return DFileService::instance()->openFileByApp(realUrl(fileUrl), app);
}

bool AVFSFileController::copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DUrlList realUrlList;
    foreach (const DUrl& url, urlList) {
        realUrlList << realUrl(url);
    }

    return DFileService::instance()->copyFilesToClipboard(realUrlList);
}

bool AVFSFileController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    return DFileService::instance()->openInTerminal(fileUrl);
}

DUrl AVFSFileController::realUrl(const DUrl &url)
{
    QString archRootPath = findArchFileRootPath(url);
    return AVFSFileInfo::realFileUrl(archRootPath, url);
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

