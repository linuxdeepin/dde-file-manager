#include "filecontroller.h"
#include "fileinfo.h"

#include "../models/desktopfileinfo.h"

#include "../controllers/filejob.h"

#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../shutil/fileutils.h"
#include "filemonitor/filemonitor.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>

class FileDirIterator : public DDirIterator
{
public:
    FileDirIterator(const QString &path,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    const AbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;

private:
    QDirIterator iterator;
};

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

bool FileController::findExecutable(const QString &executableName, const QStringList &paths)
{
    return !QStandardPaths::findExecutable(executableName, paths).isEmpty();
}

const AbstractFileInfoPointer FileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    if(fileUrl.path().endsWith(QString(".") + DESKTOP_SURRIX))
        return AbstractFileInfoPointer(new DesktopFileInfo(fileUrl));
    else
        return AbstractFileInfoPointer(new FileInfo(fileUrl));
}

const DDirIteratorPointer FileController::createDirIterator(const DUrl &fileUrl, QDir::Filters filters,
                                                            QDirIterator::IteratorFlags flags,
                                                            bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new FileDirIterator(fileUrl.path(), filters, flags));
}

bool FileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    const AbstractFileInfoPointer pfile = createFileInfo(fileUrl, accepted);

    if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
        int code = dialogManager->showRunExcutableDialog(fileUrl);

        return FileUtils::openExcutableFile(fileUrl.path(), code);
    }else if (pfile->isSymLink() && !QFile(pfile->symLinkTarget()).exists()){
        QString targetName = pfile->symLinkTarget().split("/").last();
        dialogManager->showBreakSymlinkDialog(targetName, fileUrl);
        return false;
    }

    return FileUtils::openFile(fileUrl.path());
}

bool FileController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = false;

    if (findExecutable("file-roller")){
        QStringList args;
        args << "-d";
        foreach (DUrl url, urlList) {
            args << url.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }

    return accepted;
}

bool FileController::decompressFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    if (findExecutable("file-roller")){
        QStringList args;
        args << "-f" << fileUrl.toLocalFile();
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }
    return accepted;
}

bool FileController::decompressFileHere(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    if (findExecutable("file-roller")){
        QStringList args;
        args << "-h" << fileUrl.toLocalFile();
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }
    return accepted;
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
    mimeData->setUrls(DUrl::toQUrlList(urlList));

    qApp->clipboard()->setMimeData(mimeData);

    return true;
}

bool FileController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    accepted = true;

    QFile file(oldUrl.toLocalFile());
    const QString &newFilePath = newUrl.toLocalFile();
    bool result = file.rename(newFilePath);

    if (!result) {
        result = QProcess::execute("mv \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;
    }

    return result;
}

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(event);
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
    mimeData->setUrls(DUrl::toQUrlList(urlList));

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
        FileJob job("move");

        dialogManager->addJob(&job);

        job.doMove(DUrl::toQUrlList(urlList), event.fileUrl().toString());
        dialogManager->removeJob(job.getJobId());

        qApp->clipboard()->clear();
    } else {

        FileJob job("copy");

        dialogManager->addJob(&job);

        job.doCopy(DUrl::toQUrlList(urlList), event.fileUrl().toString());
        dialogManager->removeJob(job.getJobId());
    }

    return true;
}


bool FileController::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = true;

//    qDebug() << srcUrl << tarUrl << event;
    FileJob job("restore");

    dialogManager->addJob(&job);

    job.doTrashRestore(srcUrl.path(), tarUrl.path());
    dialogManager->removeJob(job.getJobId());

    return true;
}


bool FileController::newFolder(const FMEvent &event, bool &accepted) const
{
    accepted = true;

    //Todo:: check if mkdir is ok
    QDir dir(event.fileUrl().toLocalFile());

    QString folderName = checkDuplicateName(dir.absolutePath() + "/" + tr("New Folder"));

    FileJob::SelectedFiles.insert(DUrl::fromLocalFile(folderName), event.windowId());
    return dir.mkdir(folderName);
}

bool FileController::newFile(const DUrl &toUrl, bool &accepted) const
{
    accepted = true;

    //Todo:: check if mkdir is ok
    QDir dir(toUrl.toLocalFile());
    QString name = checkDuplicateName(dir.absolutePath() + "/" + tr("New File"));

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
        DUrl parentUrl = DUrl::fromLocalFile(file.absolutePath());
        QUrlQuery query;

        query.addQueryItem("selectUrl", fileUrl.toString());
        parentUrl.setQuery(query);

        fileService->openNewWindow(parentUrl);
    } else {
        return false;
    }

    return true;
}

bool FileController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    const QString &current_dir = QDir::currentPath();

    QDir::setCurrent(fileUrl.toLocalFile());

    bool ok = QProcess::startDetached("x-terminal-emulator");

    QDir::setCurrent(current_dir);

    return ok;
}

bool FileController::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const
{
    accepted = true;

    return QFile::link(fileUrl.toLocalFile(), linkToUrl.toLocalFile());
}

void FileController::onFileCreated(const QString &filePath)
{
    DUrl url = DUrl::fromLocalFile(filePath);
    emit childrenAdded(url);
    if (FileJob::SelectedFiles.contains(url)){
        int windowId = FileJob::SelectedFiles.value(url);
        FMEvent event;
        event = windowId;
        event = url;
        emit fileSignalManager->requestSelectRenameFile(event);
        FileJob::SelectedFiles.remove(url);
    }
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

FileDirIterator::FileDirIterator(const QString &path, QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
    , iterator(path, filter, flags)
{

}

DUrl FileDirIterator::next()
{
    return DUrl::fromLocalFile(iterator.next());
}

bool FileDirIterator::hasNext() const
{
    return iterator.hasNext();
}

QString FileDirIterator::fileName() const
{
    return iterator.fileName();
}

QString FileDirIterator::filePath() const
{
    return iterator.filePath();
}

const AbstractFileInfoPointer FileDirIterator::fileInfo() const
{
    return AbstractFileInfoPointer(new FileInfo(iterator.fileInfo()));
}

QString FileDirIterator::path() const
{
    return iterator.filePath();
}
