#include "filecontroller.h"
#include "fileinfo.h"
#include "fileservices.h"

#include "../models/desktopfileinfo.h"

#include "../controllers/filejob.h"

#include "../app/global.h"
#include "../app/fmevent.h"

#include "filemonitor/filemonitor.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>

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
}

AbstractFileInfo *FileController::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    accepted = true;

    if(fileUrl.endsWith(QString(".") + DESKTOP_SURRIX))
        return new DesktopFileInfo(fileUrl);
    else
        return new FileInfo(fileUrl);
}

const QList<AbstractFileInfo*> FileController::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    accepted = true;

    QList<AbstractFileInfo*> infolist;

    const QString &path = QUrl(fileUrl).path();

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
    accepted = true;

    return QDesktopServices::openUrl(QUrl(fileUrl));
}

bool FileController::copyFiles(const QList<QString> &urlList, bool &accepted) const
{
    accepted = true;

    QMimeData *mimeData = new QMimeData;

    QByteArray ba;

    ba.append("copy");

    for(const QString &url : urlList) {
        ba.append("\n");
        ba.append(url);
    }

    mimeData->setText("copy");
    mimeData->setData("x-special/gnome-copied-files", ba);

    qApp->clipboard()->setMimeData(mimeData);

    return true;
}

bool FileController::renameFile(const QString &oldUrl, const QString &newUrl, bool &accepted) const
{
    accepted = true;

    QFile file(QUrl(oldUrl).toLocalFile());

    return file.rename(QUrl(newUrl).toLocalFile());
}

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const QList<QString> &urlList, bool &accepted) const
{
    accepted = true;

    FileJob job;

    dialogManager->addJob(&job);

    QList<QUrl> qurls;

    for(const QString &url : urlList) {
        qurls << QUrl(url);
    }

    job.doDelete(qurls);
    dialogManager->removeJob(job.getJobId());

    return true;
}

/**
 * @brief FileController::moveToTrash
 * @param urlList accepted
 *
 * Trash file or directory with the given url address.
 */
bool FileController::moveToTrash(const QList<QString> &urlList, bool &accepted) const
{
    accepted = true;

    FileJob job;

    dialogManager->addJob(&job);

    QList<QUrl> qurls;

    for(const QString &url : urlList) {
        qurls << QUrl(url);
    }

    job.doMoveToTrash(qurls);
    dialogManager->removeJob(job.getJobId());

    return true;
}

bool FileController::cutFiles(const QList<QString> &urlList, bool &accepted) const
{
    accepted = true;

    QMimeData *mimeData = new QMimeData;

    QByteArray ba;

    ba.append("cut");

    for(const QString &url : urlList) {
        ba.append("\n");
        ba.append(url);
    }

    mimeData->setText("cut");
    mimeData->setData("x-special/gnome-copied-files", ba);

    qApp->clipboard()->setMimeData(mimeData);

    return true;
}

bool FileController::pasteFile(PasteType type, const QList<QString> &urlList,
                               const FMEvent &event, bool &accepted) const
{
    accepted = true;

    QUrl localUrl(event.fileUrl());
    QDir dir(localUrl.toLocalFile());
    //Make sure the target directory exists.
    if(!dir.exists())
        return false;

    if(type == CutType) {
        for(const QString &fileUrl : urlList) {
            QFileInfo fileInfo(QUrl(fileUrl).toLocalFile());

            QFile::rename(fileInfo.absoluteFilePath(),
                          dir.absolutePath() + QDir::separator() + fileInfo.fileName());
        }
    } else {
        QList<QUrl> urls;

        for(const QString &fileUrl : urlList) {
            urls.append(QUrl(fileUrl));
        }

        FileJob job;

        dialogManager->addJob(&job);

        job.doCopy(urls, event.fileUrl());
        dialogManager->removeJob(job.getJobId());
    }

    return true;
}

bool FileController::newFolder(const QString &toUrl, bool &accepted) const
{
    accepted = true;

    QUrl localUrl(toUrl);
    //Todo:: check if mkdir is ok
    QDir dir(localUrl.toLocalFile());

    return dir.mkdir(checkDuplicateName(dir.absolutePath() + "/New Folder"));
}

bool FileController::newFile(const QString &toUrl, bool &accepted) const
{
    accepted = true;

    QUrl localUrl(toUrl);
    //Todo:: check if mkdir is ok
    QDir dir(localUrl.toLocalFile());
    QString name = checkDuplicateName(dir.absolutePath() + "/New File");

    QFile file(name);

    if(file.open(QIODevice::WriteOnly)) {
        file.close();
    } else {
        return false;
    }

    return true;
}

bool FileController::newDocument(const QString &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)
    Q_UNUSED(accepted)

    accepted = false;

    return false;
}

bool FileController::addUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->addMonitorPath(QUrl(fileUrl).toLocalFile());

    return true;
}

bool FileController::removeUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->removeMonitorPath(QUrl(fileUrl).toLocalFile());

    return true;
}

bool FileController::openFileLocation(const QString &fileUrl, bool &accepted) const
{
    accepted = true;

    QFileInfo file(QUrl(fileUrl).toLocalFile());

    if(file.exists()) {
        FMEvent event;

        event = QString(FILE_SCHEME) + "://" + file.absolutePath();
        event = FMEvent::FileView;

        fileService->openUrl(event);
    } else {
        return false;
    }

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

QString FileController::checkDuplicateName(const QString &name) const
{
    QString destUrl = name;
    QFile file(destUrl);
    QFileInfo startInfo(destUrl);

    int num = 1;

    while (file.exists()) {
        num++;
        destUrl = QString("%1/%2 %3").arg(startInfo.absolutePath()).
                arg(startInfo.fileName()).arg(num);
        file.setFileName(destUrl);
    }

    return destUrl;
}
