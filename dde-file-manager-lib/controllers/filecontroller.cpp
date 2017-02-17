#include "filecontroller.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfilewatcher.h"
#include "dfileinfo.h"
#include "models/desktopfileinfo.h"
#include "models/computerdesktopfileinfo.h"
#include "models/trashdesktopfileinfo.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"

#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"

#include "widgets/singleton.h"
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
#include "unistd.h"

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

const DAbstractFileInfoPointer FileController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    if (fileUrl.toLocalFile().endsWith(QString(".") + DESKTOP_SURRIX)){

        if(fileUrl == ComputerDesktopFileInfo::computerDesktopFileUrl())
            return DAbstractFileInfoPointer(new ComputerDesktopFileInfo());

        if(fileUrl == TrashDesktopFileInfo::trashDesktopFileUrl())
            return DAbstractFileInfoPointer(new TrashDesktopFileInfo());

        return DAbstractFileInfoPointer(new DesktopFileInfo(fileUrl));
    }

    else
        return DAbstractFileInfoPointer(new DFileInfo(fileUrl));
}

const DDirIteratorPointer FileController::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                            QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                            bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new FileDirIterator(fileUrl.toLocalFile(), nameFilters, filters, flags));
}

bool FileController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    const DAbstractFileInfoPointer pfile = createFileInfo(fileUrl, accepted);

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(pfile->symLinkTarget());

        if (linkInfo && !linkInfo->exists()) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
            return false;
        }
        const_cast<DUrl&>(fileUrl) = linkInfo->redirectedFileUrl();
    }

    if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
        int code = dialogManager->showRunExcutableScriptDialog(fileUrl);
        return FileUtils::openExcutableScriptFile(fileUrl.toLocalFile(), code);
    }

    if(FileUtils::istFileRunnable(fileUrl.toLocalFile()) && !pfile->isDesktopFile()){
        int code = dialogManager->showRunExcutableFileDialog(fileUrl);
        return FileUtils::openExcutableFile(fileUrl.toLocalFile(), code);
    }

    return FileUtils::openFile(fileUrl.toLocalFile());
}

bool FileController::openFileByApp(const DUrl &fileUrl, const QString& app, bool &accepted) const
{
    accepted = true;
    return FileUtils::openFileByApp(fileUrl.toLocalFile(), app);
}

bool FileController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

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

bool FileController::decompressFile(const DUrlList &fileUrlList, bool &accepted) const{
    accepted = true;
    if (findExecutable("file-roller")){
        QStringList args;
        args << "-f";
        for(auto it : fileUrlList){
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }
    return accepted;
}

bool FileController::decompressFileHere(const DUrlList &fileUrlList, bool &accepted) const{
    accepted = true;
    if (findExecutable("file-roller")){
        QStringList args;
        args << "-h";
        for(auto it : fileUrlList){
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    }else{
        qDebug() << "file-roller is not installed";
    }
    return accepted;
}

bool FileController::copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(urlList), DFMGlobal::CopyAction);

    return true;
}

bool FileController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    accepted = true;

    QFile file(oldUrl.toLocalFile());
    const QString &newFilePath = newUrl.toLocalFile();

    const DAbstractFileInfoPointer oldfilePointer = createFileInfo(oldUrl, accepted);
    const DAbstractFileInfoPointer newfilePointer = createFileInfo(newUrl, accepted);

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
bool FileController::deleteFiles(const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(event);
    accepted = true;

    FileJob job(FileJob::Delete);

    dialogManager->addJob(&job);

    job.doDelete(event.fileUrlList());
    dialogManager->removeJob(job.getJobId());

    return true;
}

/**
 * @brief FileController::moveToTrash
 * @param urlList accepted
 *
 * Trash file or directory with the given url address.
 */
DUrlList FileController::moveToTrash(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    FileJob job(FileJob::Delete);

    dialogManager->addJob(&job);

    DUrlList list = job.doMoveToTrash(event.fileUrlList());
    dialogManager->removeJob(job.getJobId());

    return list;
}

bool FileController::cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(urlList), DFMGlobal::CutAction);

    return true;
}

DUrlList FileController::pasteFile(PasteType type, const DUrl &targetUrl, const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    const DUrlList &urlList = event.fileUrlList();

    if (urlList.isEmpty())
        return DUrlList();

    DUrlList list;
    QDir dir(targetUrl.toLocalFile());
    //Make sure the target directory exists.
    if(!dir.exists())
        return list;

    if (type == CutType) {
        DUrl parentUrl = DUrl::parentUrl(urlList.first());

        if (parentUrl != targetUrl) {
            FileJob job(FileJob::Move);

            dialogManager->addJob(&job);

            list = job.doMove(urlList, targetUrl);
            dialogManager->removeJob(job.getJobId());
        }

        DFMGlobal::clearClipboard();
    } else {

        FileJob job(FileJob::Copy);

        dialogManager->addJob(&job);

        list = job.doCopy(urlList, targetUrl);
        dialogManager->removeJob(job.getJobId());
    }

    return list;
}


bool FileController::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = true;

//    qDebug() << srcUrl << tarUrl << event;
    FileJob job(FileJob::Restore);

    dialogManager->addJob(&job);

    job.doTrashRestore(srcUrl.path(), tarUrl.path());
    dialogManager->removeJob(job.getJobId());

    return true;
}

bool FileController::newFolder(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    //Todo:: check if mkdir is ok
    QDir dir(event.fileUrl().toLocalFile());

    QString folderName = checkDuplicateName(dir.absolutePath() + "/" + tr("New Folder"));

    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(folderName), event.windowId());
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

bool FileController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    QFileInfo file(fileUrl.toLocalFile());

    if(file.exists()) {
        DUrl parentUrl = DUrl::fromLocalFile(file.absolutePath());
        QUrlQuery query;

        query.addQueryItem("selectUrl", fileUrl.toString());
        parentUrl.setQuery(query);

        DFMEvent event;
        DUrlList urlList;
        urlList << parentUrl;
        event << urlList;
        event << parentUrl;
        fileService->openNewWindow(event, true);
    } else {
        return false;
    }

    return true;
}

bool FileController::unShareFolder(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    const ShareInfo& info = userShareManager->getShareInfoByPath(fileUrl.path());
    userShareManager->deleteUserShare(info);

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

    bool ok;
    int code = ::symlink(fileUrl.toLocalFile().toStdString().data(), linkToUrl.toLocalFile().toStdString().data());
    if(code == -1){
        ok = false;
        QString errorString = strerror(errno);
        dialogManager->showFailToCreateSymlinkDialog(errorString);
    } else{
        ok = true;
    }

    return ok;
}

DAbstractFileWatcher *FileController::createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const
{
    accepted = true;

    return new DFileWatcher(fileUrl.toLocalFile(), parent);
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

        #ifdef DDE_COMPUTER_TRASH
        if(filePath() == ComputerDesktopFileInfo::computerDesktopFileUrl().toLocalFile())
            return DAbstractFileInfoPointer(new ComputerDesktopFileInfo());
        else if(filePath() == TrashDesktopFileInfo::trashDesktopFileUrl().toLocalFile())
            return DAbstractFileInfoPointer(new TrashDesktopFileInfo());
        #endif

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
