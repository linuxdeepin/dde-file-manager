#include "filecontroller.h"
#include "fileinfo.h"

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

    connect(fileMonitor, &FileMonitor::fileCreated,
            this, &FileController::onFileCreated);
    connect(fileMonitor, &FileMonitor::fileDeleted,
            this, &FileController::onFileRemove);
}

AbstractFileInfo *FileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    if(fileUrl.path().endsWith(QString(".") + DESKTOP_SURRIX))
        return new DesktopFileInfo(fileUrl);
    else
        return new FileInfo(fileUrl);
}

const QList<AbstractFileInfo*> FileController::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    accepted = true;

    QList<AbstractFileInfo*> infolist;

    const QString &path = fileUrl.toLocalFile();

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

bool FileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return QDesktopServices::openUrl(fileUrl);
}

bool FileController::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    QMimeData *mimeData = new QMimeData;

    QByteArray ba;

    ba.append("copy");

    for(const DUrl &url : urlList) {
        ba.append("\n");
        ba.append(url.toString());
    }

    mimeData->setText("copy");
    mimeData->setData("x-special/gnome-copied-files", ba);

    qApp->clipboard()->setMimeData(mimeData);

    return true;
}

bool FileController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    accepted = true;

    QFile file(oldUrl.toLocalFile());

    return file.rename(newUrl.toLocalFile());
}

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    FileJob job("delete");

    dialogManager->addJob(&job);

    job.doDelete(DUrl::toQUrlList(urlList));
    dialogManager->removeJob(job.getJobId());

    return true;
}

/**
 * @brief FileController::moveToTrash
 * @param urlList accepted
 *
 * Trash file or directory with the given url address.
 */
bool FileController::moveToTrash(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    FileJob job("delete");

    dialogManager->addJob(&job);

    job.doMoveToTrash(DUrl::toQUrlList(urlList));
    dialogManager->removeJob(job.getJobId());

    return true;
}

bool FileController::cutFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    QMimeData *mimeData = new QMimeData;

    QByteArray ba;

    ba.append("cut");

    for(const DUrl &url : urlList) {
        ba.append("\n");
        ba.append(url.toString());
    }

    mimeData->setText("cut");
    mimeData->setData("x-special/gnome-copied-files", ba);

    qApp->clipboard()->setMimeData(mimeData);

    return true;
}

bool FileController::pasteFile(PasteType type, const DUrlList &urlList,
                               const FMEvent &event, bool &accepted) const
{
    accepted = true;

    QDir dir(event.fileUrl().toLocalFile());
    //Make sure the target directory exists.
    if(!dir.exists())
        return false;

    if(type == CutType) {
        for(const DUrl &fileUrl : urlList) {
            QFileInfo fileInfo(fileUrl.toLocalFile());

            QFile::rename(fileInfo.absoluteFilePath(),
                          dir.absolutePath() + QDir::separator() + fileInfo.fileName());
        }
    } else {

        FileJob job("copy");

        dialogManager->addJob(&job);

        job.doCopy(DUrl::toQUrlList(urlList), event.fileUrl().toString());
        dialogManager->removeJob(job.getJobId());
    }

    return true;
}

bool FileController::newFolder(const DUrl &toUrl, bool &accepted) const
{
    accepted = true;

    //Todo:: check if mkdir is ok
    QDir dir(toUrl.toLocalFile());

    return dir.mkdir(checkDuplicateName(dir.absolutePath() + "/New Folder"));
}

bool FileController::newFile(const DUrl &toUrl, bool &accepted) const
{
    accepted = true;

    //Todo:: check if mkdir is ok
    QDir dir(toUrl.toLocalFile());
    QString name = checkDuplicateName(dir.absolutePath() + "/New File");

    QFile file(name);

    if(file.open(QIODevice::WriteOnly)) {
        file.close();
    } else {
        return false;
    }

    return true;
}

bool FileController::newDocument(const DUrl &toUrl, bool &accepted) const
{
    Q_UNUSED(toUrl)
    Q_UNUSED(accepted)

    accepted = false;

    return false;
}

bool FileController::addUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->addMonitorPath(fileUrl.toLocalFile());

    return true;
}

bool FileController::removeUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->removeMonitorPath(fileUrl.toLocalFile());

    return true;
}

bool FileController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    QFileInfo file(fileUrl.toLocalFile());

    if(file.exists()) {
        FMEvent event;

        event = DUrl::fromLocalFile(file.absolutePath());
        event = FMEvent::FileView;

        fileService->openUrl(event);
    } else {
        return false;
    }

    return true;
}

void FileController::onFileCreated(const QString &filePath)
{
    emit childrenAdded(DUrl::fromLocalFile(filePath));
}

void FileController::onFileRemove(const QString &filePath)
{
    emit childrenRemoved(DUrl::fromLocalFile(filePath));
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
