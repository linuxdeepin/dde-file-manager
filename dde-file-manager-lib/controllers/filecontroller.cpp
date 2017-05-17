#include "filecontroller.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfilewatcher.h"
#include "dfileinfo.h"
#include "trashmanager.h"
#include "models/desktopfileinfo.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"

#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"

#include "singleton.h"
#include "interfaces/dfmglobal.h"

#include "appcontroller.h"

#include "models/sharefileinfo.h"
#include "usershare/usersharemanager.h"
#include "dfmsetting.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QGuiApplication>
#include <QUrlQuery>

#include <unistd.h>

class FileDirIterator : public DDirIterator
{
public:
    FileDirIterator(const QString &path,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    ~FileDirIterator();

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;

    bool enableIteratorByKeyword(const QString &keyword) Q_DECL_OVERRIDE;

private:
    QDirIterator iterator;
    QProcess *processRlocate = Q_NULLPTR;
    QFileInfo currentFileInfo;
};

FileController::FileController(QObject *parent)
    : DAbstractFileController(parent)
{
    qRegisterMetaType<QList<DFileInfo*>>(QT_STRINGIFY(QList<DFileInfo*>));
}

bool FileController::findExecutable(const QString &executableName, const QStringList &paths)
{
    return !QStandardPaths::findExecutable(executableName, paths).isEmpty();
}

const DAbstractFileInfoPointer FileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    if (event->url().toLocalFile().endsWith(QString(".") + DESKTOP_SURRIX)){

        return DAbstractFileInfoPointer(new DesktopFileInfo(event->url()));
    }

    return DAbstractFileInfoPointer(new DFileInfo(event->url()));
}

const DDirIteratorPointer FileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new FileDirIterator(event->url().toLocalFile(), event->nameFilters(),
                                                   event->filters(), event->flags()));
}

bool FileController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    DUrl fileUrl = event->url();

    const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvnet>(this, fileUrl));

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

        if (linkInfo && !linkInfo->exists()) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
            return false;
        }
        const_cast<DUrl&>(fileUrl) = linkInfo->redirectedFileUrl();
    }

    if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
        int code = dialogManager->showRunExcutableScriptDialog(fileUrl, event->windowId());
        return FileUtils::openExcutableScriptFile(fileUrl.toLocalFile(), code);
    }

    if(FileUtils::isFileRunnable(fileUrl.toLocalFile()) && !pfile->isDesktopFile()){
        int code = dialogManager->showRunExcutableFileDialog(fileUrl, event->windowId());
        return FileUtils::openExcutableFile(fileUrl.toLocalFile(), code);
    }

    return FileUtils::openFile(fileUrl.toLocalFile());
}

bool FileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return FileUtils::openFileByApp(event->url().toLocalFile(), event->appName());
}

bool FileController::compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const
{
    if (findExecutable("file-roller")) {
        QStringList args;
        args << "-d";
        foreach (DUrl url, event->urlList()) {
            args << url.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    } else {
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    if (findExecutable("file-roller")) {
        QStringList args;
        args << "-f";
        for(auto it : event->urlList()) {
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    if (findExecutable("file-roller")) {
        QStringList args;
        args << "-h";
        for(auto it : event->urlList()) {
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(event->urlList()), event->action());

    return true;
}

bool FileController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const DUrl &oldUrl = event->fromUrl();
    const DUrl &newUrl = event->toUrl();

    QFile file(oldUrl.toLocalFile());
    const QString &newFilePath = newUrl.toLocalFile();

    const DAbstractFileInfoPointer &oldfilePointer = DFileService::instance()->createFileInfo(this, oldUrl);
    const DAbstractFileInfoPointer &newfilePointer = DFileService::instance()->createFileInfo(this, newUrl);

    bool result(false);

    if (oldfilePointer->isDesktopFile()){
        QString filePath = oldUrl.toLocalFile();
        Properties desktop(filePath, "Desktop Entry");
        QString key;
        QString localKey = QString("Name[%1]").arg(QLocale::system().name());
        if (desktop.contains(localKey)){
            key = localKey;
        }else{
            key = "Name";
        }
        desktop.set(key, newfilePointer->fileName());
        result = desktop.save(filePath, "Desktop Entry");
    }else{
        result = file.rename(newFilePath);

        if (!result) {
            result = QProcess::execute("mv \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;
        }
    }

    return result;
}

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    FileJob job(FileJob::Delete);

    dialogManager->addJob(&job);

    job.doDelete(event->urlList());
    dialogManager->removeJob(job.getJobId());

    return true;
}

/**
 * @brief FileController::moveToTrash
 * @param urlList accepted
 *
 * Trash file or directory with the given url address.
 */
DUrlList FileController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    FileJob job(FileJob::Trash);

    dialogManager->addJob(&job);

    DUrlList list = job.doMoveToTrash(event->urlList());
    dialogManager->removeJob(job.getJobId());

    return list;
}

DUrlList FileController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    const DUrlList &urlList = event->urlList();

    if (urlList.isEmpty())
        return DUrlList();

    DUrlList list;
    QDir dir(event->targetUrl().toLocalFile());
    //Make sure the target directory exists.
    if(!dir.exists())
        return list;

    if (event->action() == DFMGlobal::CutAction) {
        DUrl parentUrl = DUrl::parentUrl(urlList.first());

        if (parentUrl != event->targetUrl()) {
            FileJob job(FileJob::Move);

            dialogManager->addJob(&job);

            list = job.doMove(urlList, event->targetUrl());
            dialogManager->removeJob(job.getJobId());
        }

        DFMGlobal::clearClipboard();
    } else {

        FileJob job(FileJob::Copy);

        dialogManager->addJob(&job);

        list = job.doCopy(urlList, event->targetUrl());
        dialogManager->removeJob(job.getJobId());
    }

    return list;
}

bool FileController::newFolder(const QSharedPointer<DFMNewFolderEvent> &event) const
{
    //Todo:: check if mkdir is ok
    QDir dir(event->url().toLocalFile());

    QString folderName = checkDuplicateName(dir.absolutePath() + "/" + tr("New Folder"));

    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(folderName), event->windowId());
    return dir.mkdir(folderName);
}

bool FileController::newFile(const QSharedPointer<DFMNewFileEvent> &event) const
{
    //Todo:: check if mkdir is ok
    QDir dir(event->url().toLocalFile());
    QString name = checkDuplicateName(dir.absolutePath() + "/" + tr("New File") + event->fileSuffix());

    QFile file(name);

    if(file.open(QIODevice::WriteOnly)) {
        file.close();
    } else {
        return false;
    }

    return true;
}

bool FileController::shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const
{
    ShareInfo info;
    info.setPath(event->url().toLocalFile());

    info.setShareName(event->name());
    info.setIsGuestOk(event->allowGuest());
    info.setIsWritable(event->isWritable());

    userShareManager->addUserShare(info);

    return true;
}

bool FileController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    userShareManager->deleteUserShareByPath(event->url().toLocalFile());

    return true;
}

bool FileController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    const QString &current_dir = QDir::currentPath();

    QDir::setCurrent(event->url().toLocalFile());

    bool ok = QProcess::startDetached("x-terminal-emulator");

    QDir::setCurrent(current_dir);

    return ok;
}

bool FileController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    QFile file(event->fileUrl().toLocalFile());

    bool ok = file.link(event->toUrl().toLocalFile());

    if (ok)
        return true;

    int code = ::symlink(event->fileUrl().toLocalFile().toLocal8Bit().constData(),
                         event->toUrl().toLocalFile().toLocal8Bit().constData());
    if (code == -1) {
        ok = false;
        QString errorString = strerror(errno);
        dialogManager->showFailToCreateSymlinkDialog(errorString);
    } else {
        ok = true;
    }

    return ok;
}

DAbstractFileWatcher *FileController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFileWatcher(event->url().toLocalFile());
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

FileDirIterator::FileDirIterator(const QString &path, const QStringList &nameFilters,
                                 QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
    , iterator(path, nameFilters, filter, flags)
{

}

FileDirIterator::~FileDirIterator()
{
    if (processRlocate) {
        processRlocate->kill();
        processRlocate->terminate();
        processRlocate->waitForFinished();
        processRlocate->deleteLater();
    }
}

DUrl FileDirIterator::next()
{
    if (!processRlocate)
        return DUrl::fromLocalFile(iterator.next());

    processRlocate->waitForReadyRead();
    QString filePath = processRlocate->readLine();

    if (filePath.isEmpty()) {
        return DUrl();
    }

    filePath.chop(1);

    currentFileInfo.setFile(filePath);

    return DUrl::fromLocalFile(filePath);
}

bool FileDirIterator::hasNext() const
{
    if (!processRlocate)
        return iterator.hasNext();

    return processRlocate->state() != QProcess::NotRunning || processRlocate->canReadLine();
}

QString FileDirIterator::fileName() const
{
    if (!processRlocate)
        return iterator.fileName();

    return currentFileInfo.fileName();
}

QString FileDirIterator::filePath() const
{
    if (!processRlocate)
        return iterator.filePath();

    return currentFileInfo.filePath();
}

const DAbstractFileInfoPointer FileDirIterator::fileInfo() const
{
    if (fileName().contains(QChar(0xfffd))) {
        DFMGlobal::fileNameCorrection(filePath());
    }

    if (fileName().endsWith(QString(".") + DESKTOP_SURRIX)){

        return DAbstractFileInfoPointer(new DesktopFileInfo(processRlocate ? currentFileInfo : iterator.fileInfo()));
    }
    return DAbstractFileInfoPointer(new DFileInfo(processRlocate ? currentFileInfo : iterator.fileInfo()));
}

QString FileDirIterator::path() const
{
    return iterator.path();
}

bool FileDirIterator::enableIteratorByKeyword(const QString &keyword)
{
    if (!globalSetting->isQuickSearch()){
        return false;
    }

    if (processRlocate)
        return true;

    QProcess process;

    process.closeReadChannel(QProcess::StandardError);
    process.closeReadChannel(QProcess::StandardOutput);
    process.start("which rlocate");
    process.waitForFinished();

    if (process.exitCode() == 0 && !keyword.isEmpty()) {
        QString arg = path() + QString(".*%1[^/]*$").arg(keyword);

        processRlocate = new QProcess();
        processRlocate->start("rlocate", QStringList() << "-r" << arg << "-i", QIODevice::ReadOnly);

        return true;
    }

    return false;
}
