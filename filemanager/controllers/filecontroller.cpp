#include "filecontroller.h"
#include "fileinfo.h"
#include "fileservices.h"

#include "../models/desktopfileinfo.h"

#include "filemonitor/filemonitor.h"

#include <QDesktopServices>
#include <QDirIterator>

FileController::FileController(QObject *parent)
    : AbstractFileController(parent)
    , fileMonitor(new FileMonitor(this))
{
    qRegisterMetaType<QList<FileInfo*>>("QList<FileInfo*>");

    FileServices::setFileUrlHandler("file", "", this);
    FileServices::setFileUrlHandler("", "", this);

    connect(fileMonitor, &FileMonitor::fileCreated,
            this, &FileController::onFileCreated);
    connect(fileMonitor, &FileMonitor::fileDeleted,
            this, &FileController::onFileRemove);
    connect(fileMonitor, &FileMonitor::fileRenamed,
            this, &FileController::onFileRename);
}

AbstractFileInfo *FileController::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(!url.isLocalFile()) {
        accepted = false;

        return Q_NULLPTR;
    }

    accepted = true;

    if(fileUrl.endsWith(QString(".") + DESKTOP_SURRIX))
        return new DesktopFileInfo(fileUrl);
    else
        return new FileInfo(fileUrl);
}

const QList<AbstractFileInfo*> FileController::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    QUrl url(fileUrl);

    if(!url.isLocalFile()) {
        accepted = false;
    }

    accepted = true;

    QList<AbstractFileInfo*> infolist;

    const QString &path = url.path();

    if(path.isEmpty()) {
        const QFileInfoList list = QDir::drives();

        for(const QFileInfo &info : list) {
            FileInfo *fileInfo = new FileInfo(info);

            infolist.append(fileInfo);
        }
    } else {
        QDirIterator dirIt(path, filter);
        FileInfo *fileInfo;

        while (dirIt.hasNext()) {
            dirIt.next();

            if(dirIt.fileInfo().absoluteFilePath() == path)
                continue;

            if(dirIt.fileInfo().suffix() == DESKTOP_SURRIX)
                fileInfo = new DesktopFileInfo(dirIt.fileInfo());
            else
                fileInfo = new FileInfo(dirIt.fileInfo());

            infolist.append(fileInfo);
        }
    }

    return infolist;
}

bool FileController::openFile(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(!url.isLocalFile()) {
        accepted = false;

        return false;
    }

    accepted = true;

    return QDesktopServices::openUrl(url);
}

bool FileController::renameFile(const QString &oldUrl, const QString &newUrl, bool &accepted) const
{
    QUrl url(oldUrl);

    if(!url.isLocalFile()) {
        accepted = false;

        return false;
    }

    accepted = true;

    QFile file(url.toLocalFile());

    return file.rename(QUrl(newUrl).toLocalFile());
}

bool FileController::addUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(!url.isLocalFile()) {
        accepted = false;

        return false;
    }

    accepted = true;

    fileMonitor->addMonitorPath(url.toLocalFile());

    return true;
}

bool FileController::removeUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(!url.isLocalFile()) {
        accepted = false;

        return false;
    }

    accepted = true;

    fileMonitor->removeMonitorPath(url.toLocalFile());

    return true;
}

void FileController::onFileCreated(const QString &filePath)
{
    emit childrenAdded(QUrl::fromLocalFile(filePath).toString());
}

void FileController::onFileRemove(const QString &filePath)
{
    emit childrenRemoved(QUrl::fromLocalFile(filePath).toString());
}

void FileController::onFileRename(const QString &oldName, const QString &newName)
{
    onFileRemove(oldName);
    onFileCreated(newName);
}
