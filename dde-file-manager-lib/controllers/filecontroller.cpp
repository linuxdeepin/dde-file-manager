/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filecontroller.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfilewatcher.h"
#include "dfileinfo.h"
#include "trashmanager.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dlocalfiledevice.h"
#include "dgiofiledevice.h"
#include "dlocalfilehandler.h"
#include "dfilecopymovejob.h"
#include "dstorageinfo.h"

#include "models/desktopfileinfo.h"
#include "models/trashfileinfo.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "tag/tagmanager.h"

#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"

#include "singleton.h"
#include "interfaces/dfmglobal.h"

#include "appcontroller.h"
#include "singleton.h"

#include "models/sharefileinfo.h"
#include "usershare/usersharemanager.h"

#include "fileoperations/sort.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QGuiApplication>
#include <QUrlQuery>
#include <QRegularExpression>

#include <unistd.h>

#include <QQueue>

#include "dfmsettings.h"
#include "dfmapplication.h"
#ifndef DISABLE_QUICK_SEARCH
#include "quick_search/dquicksearch.h"
#endif
#include "controllers/quicksearchdaemoncontroller.h"

class DFMQDirIterator : public DDirIterator
{
public:
    DFMQDirIterator(const QString &path,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags)
        : iterator(path, nameFilters, filter, flags)
    {

    }

    DUrl next() override
    {
        return DUrl::fromLocalFile(iterator.next());
    }

    bool hasNext() const override
    {
        return iterator.hasNext();
    }

    QString fileName() const override
    {
        return iterator.fileName();
    }

    DUrl fileUrl() const override
    {
        return DUrl::fromLocalFile(iterator.filePath());
    }

    const DAbstractFileInfoPointer fileInfo() const override
    {
        const QFileInfo &info = iterator.fileInfo();

        if (info.suffix() == DESKTOP_SURRIX) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(info));
        }

        return DAbstractFileInfoPointer(new DFileInfo(info));
    }

    DUrl url() const override
    {
        return DUrl::fromLocalFile(iterator.path());
    }

private:
    QDirIterator iterator;
};

class DFMSortInodeDirIterator : public DDirIterator
{
public:
    DFMSortInodeDirIterator(const QString &path)
        : dir(path)
    {

    }

    ~DFMSortInodeDirIterator()
    {
        if (sortFiles) {
            free(sortFiles);
        } else if (sortFilesIndex) {
            delete sortFilesIndex;
        }
    }

    DUrl next() override
    {
        const QByteArray name(sortFilesIndex);

        currentFileInfo.setFile(dir.absoluteFilePath(QFile::decodeName(name)));
        sortFilesIndex += name.length() + 1;

        return DUrl::fromLocalFile(currentFileInfo.absoluteFilePath());
    }

    bool hasNext() const override
    {
        if (!sortFilesIndex) {
            sortFiles = savedir(QFile::encodeName(dir.absolutePath()).constData());

            if (sortFiles) {
                sortFilesIndex = sortFiles;
            } else {
                sortFilesIndex = new char(0);
            }
        }

        return *sortFilesIndex;
    }

    QString fileName() const override
    {
        return currentFileInfo.fileName();
    }

    DUrl fileUrl() const override
    {
        return DUrl::fromLocalFile(currentFileInfo.filePath());
    }

    const DAbstractFileInfoPointer fileInfo() const override
    {
        return DAbstractFileInfoPointer(new DFileInfo(currentFileInfo));
    }

    DUrl url() const override
    {
        return DUrl::fromLocalFile(dir.absolutePath());
    }

private:
    QDir dir;
    mutable char *sortFiles = nullptr;
    mutable char *sortFilesIndex = nullptr;
    QFileInfo currentFileInfo;
};

class DFMQuickSearchDirIterator : public DDirIterator
{
public:
    DFMQuickSearchDirIterator(const QString &path, const QString &keyword)
        : m_pathForSearching(path)
        , m_keyword(keyword)
    {

    }

    DUrl next() override
    {
        QString searched_result{ m_searchedResult.takeFirst() };
        currentFileInfo.setFile(searched_result);

        return DUrl::fromLocalFile(currentFileInfo.absoluteFilePath());
    }

    bool hasNext() const override
    {
        ///###: jundge whether had invoked quick-search and cached results or not.
        ///###: the result is not empty.
        if (m_cachedFlag.load(std::memory_order_consume) && !m_searchedResult.isEmpty()) {
            return true;
        }

        ///###: if quick-search-daemon is not ready last time.
        ///###: check out the status of quick-searh-daemon again here.
        ///###: if ready, invoke quick-search-daemon to search files.
        bool whether_cached_completely{ QuickSearchDaemonController::instance()->whetherCacheCompletely() };

        if (whether_cached_completely && !m_cachedFlag.load(std::memory_order_consume)) {
            m_searchedResult = QuickSearchDaemonController::instance()->search(m_pathForSearching, m_keyword);
            m_cachedFlag.store(true, std::memory_order_release);
        }

        return (m_cachedFlag.load(std::memory_order_consume) && !m_searchedResult.isEmpty());
    }

    QString fileName() const override
    {
        return currentFileInfo.fileName();
    }

    DUrl fileUrl() const override
    {
        return DUrl::fromLocalFile(currentFileInfo.filePath());
    }

    const DAbstractFileInfoPointer fileInfo() const override
    {
        if (currentFileInfo.suffix() == DESKTOP_SURRIX) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(currentFileInfo));
        }

        return DAbstractFileInfoPointer(new DFileInfo(currentFileInfo));
    }

    DUrl url() const override
    {
        return DUrl::fromLocalFile(m_pathForSearching);
    }

private:
    mutable std::atomic<bool> m_cachedFlag{ false };
    mutable QList<QString> m_searchedResult{};
    QString m_pathForSearching{};
    QString m_keyword;
    QFileInfo currentFileInfo;
};

class FileDirIterator : public DDirIterator
{
public:
    FileDirIterator(const QString &url,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    ~FileDirIterator();

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    DUrl url() const Q_DECL_OVERRIDE;

    bool enableIteratorByKeyword(const QString &keyword) Q_DECL_OVERRIDE;

private:
    DDirIterator *iterator = nullptr;
    bool nextIsCached = false;
    QDir::Filters filters;
};

FileController::FileController(QObject *parent)
    : DAbstractFileController(parent)
{
    qRegisterMetaType<QList<DFileInfo *>>(QT_STRINGIFY(QList<DFileInfo *>));
}

bool FileController::findExecutable(const QString &executableName, const QStringList &paths)
{
    return !QStandardPaths::findExecutable(executableName, paths).isEmpty();
}

const DAbstractFileInfoPointer FileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    if (event->url().toLocalFile().endsWith(QString(".") + DESKTOP_SURRIX)) {

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
        const_cast<DUrl &>(fileUrl) = linkInfo->redirectedFileUrl();
    }

    if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
        int code = dialogManager->showRunExcutableScriptDialog(fileUrl, event->windowId());
        return FileUtils::openExcutableScriptFile(fileUrl.toLocalFile(), code);
    }

    if (FileUtils::isFileRunnable(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
        int code = dialogManager->showRunExcutableFileDialog(fileUrl, event->windowId());
        return FileUtils::openExcutableFile(fileUrl.toLocalFile(), code);
    }

    if (FileUtils::isFileWindowsUrlShortcut(fileUrl.toLocalFile())) {
        QString url = FileUtils::getInternetShortcutUrl(fileUrl.toLocalFile());
        return FileUtils::openFile(url);
    }


    return FileUtils::openFile(fileUrl.toLocalFile());
}

bool FileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return FileUtils::openFileByApp(event->appName(), event->url().toString());
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
        for (auto it : event->urlList()) {
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    } else {
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    if (findExecutable("file-roller")) {
        QStringList args;
        args << "-h";
        for (auto it : event->urlList()) {
            args << it.toLocalFile();
        }
        qDebug() << args;
        bool result = QProcess::startDetached("file-roller", args);
        return result;
    } else {
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

    if (oldfilePointer->isDesktopFile() && !oldfilePointer->isSymLink()) {
        QString filePath = oldUrl.toLocalFile();
        Properties desktop(filePath, "Desktop Entry");
        QString key;
        QString localKey = QString("Name[%1]").arg(QLocale::system().name());
        if (desktop.contains(localKey)) {
            key = localKey;
        } else {
            key = "Name";
        }

        const QString old_name = desktop.value(key).toString();

        desktop.set(key, newfilePointer->fileName());
        result = desktop.save(filePath, "Desktop Entry");

        if (result) {
            const QString path = QFileInfo(file).absoluteDir().absoluteFilePath(old_name);

            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRenameEvent>(nullptr, oldUrl, DUrl::fromLocalFile(path)));
        }
    } else {
        result = file.rename(newFilePath);

        if (!result) {
            result = QProcess::execute("mv \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;
        }

        if (result) {
            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRenameEvent>(nullptr, newUrl, oldUrl));
        }
    }

    return result;
}

static DUrlList pasteFilesV2(DFMGlobal::ClipboardAction action, const DUrlList &list, const DUrl &target, bool slient = false, bool force = false)
{
    DFileCopyMoveJob *job = new DFileCopyMoveJob();
    QPair<DUrl, DUrl> currentJob;

    if (force) {
        job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    }

    if (action == DFMGlobal::CutAction && !target.isValid()) {
        // for remove mode
        job->setActionOfErrorType(DFileCopyMoveJob::NonexistenceError, DFileCopyMoveJob::SkipAction);
    }

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }

    QObject::connect(job, &DFileCopyMoveJob::currentJobChanged, job, [&](const DUrl & from, const DUrl & to) {
        currentJob.first = from;
        currentJob.second = to;
    }, Qt::DirectConnection);

    class ErrorHandle : public QObject, public DFileCopyMoveJob::Handle
    {
    public:
        ErrorHandle(DFileCopyMoveJob *job, QPair<DUrl, DUrl> *jobInfo, bool s)
            : QObject(nullptr)
            , slient(s)
            , fileJob(job)
            , currentJob(jobInfo)
        {
            if (!slient) {
                timer_id = startTimer(1000);
                moveToThread(qApp->thread());
            }
        }

        ~ErrorHandle()
        {
            if (timer_id > 0) {
                killTimer(timer_id);
            }
        }

        // 处理任务对话框显示之前的错误, 无法处理的错误将立即弹出对话框处理
        DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                             const DAbstractFileInfo *sourceInfo,
                                             const DAbstractFileInfo *targetInfo) override
        {
            if (slient) {
                return DFileCopyMoveJob::SkipAction;
            }

            if (error == DFileCopyMoveJob::DirectoryExistsError || error == DFileCopyMoveJob::FileExistsError) {
                if (sourceInfo->fileUrl() == targetInfo->fileUrl()) {
                    return DFileCopyMoveJob::CoexistAction;
                }
            }

            if (timer_id > 0) {
                killTimer(timer_id);
                timer_id = 0;
            }

            MoveCopyTaskWidget *taskWidget = dialogManager->taskDialog()->addTaskJob(job);

            taskWidget->onJobCurrentJobChanged(currentJob->first, currentJob->second);

            return taskWidget->errorHandle()->handleError(job, error, sourceInfo, targetInfo);
        }

        void timerEvent(QTimerEvent *e) override
        {
            if (e->timerId() != timer_id) {
                return QObject::timerEvent(e);
            }

            killTimer(timer_id);
            timer_id = 0;

            // 此时说明pasteFilesV2函数已经结束
            if (!fileJob || !currentJob)
                return;

            MoveCopyTaskWidget *taskWidget = dialogManager->taskDialog()->addTaskJob(fileJob);

            taskWidget->onJobCurrentJobChanged(currentJob->first, currentJob->second);
        }

        int timer_id = 0;
        bool slient;
        DFileCopyMoveJob *fileJob;
        QPair<DUrl, DUrl> *currentJob;
    };

    ErrorHandle *error_handle = new ErrorHandle(job, &currentJob, slient);

    job->setErrorHandle(error_handle, slient ? nullptr : error_handle->thread());
    job->setMode(action == DFMGlobal::CopyAction ? DFileCopyMoveJob::CopyMode : DFileCopyMoveJob::MoveMode);
    job->start(list, target);
    job->wait();

    QTimer::singleShot(200, dialogManager->taskDialog(), [job] {
        dialogManager->taskDialog()->removeTaskJob(job);
    });

    error_handle->currentJob = nullptr;
    error_handle->fileJob = nullptr;

    if (slient) {
        error_handle->deleteLater();
    } else {
        QMetaObject::invokeMethod(error_handle, "deleteLater");
    }

    QMetaObject::invokeMethod(job, "deleteLater");

    return job->targetUrlList();
}

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
//    FileJob job(FileJob::Delete);
//    job.setWindowId(event->windowId());
//    dialogManager->addJob(&job);

//    job.doDelete(event->urlList());
//    dialogManager->removeJob(job.getJobId());

    bool ok = !pasteFilesV2(DFMGlobal::CutAction, event->fileUrlList(), DUrl(), event->silent(), event->force()).isEmpty();

    return ok;
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
    job.setWindowId(event->windowId());
    dialogManager->addJob(&job);

    DUrlList list = job.doMoveToTrash(event->urlList());
    dialogManager->removeJob(job.getJobId());

    // save event
    const QVariant &result = DFMEventDispatcher::instance()->processEvent<DFMGetChildrensEvent>(event->sender(), DUrl::fromTrashFile("/"),
                             QStringList(), QDir::AllEntries);
    const QList<DAbstractFileInfoPointer> &infos = qvariant_cast<QList<DAbstractFileInfoPointer>>(result);

    if (infos.isEmpty()) {
        return list;
    }

    const QSet<DUrl> &source_files_set = event->urlList().toSet();
    const QSet<DUrl> &target_files_set = list.toSet();
    DUrlList has_restore_files;

    for (const DAbstractFileInfoPointer &info : infos) {
        const DUrl &source_file = DUrl::fromLocalFile(static_cast<const TrashFileInfo *>(info.constData())->sourceFilePath());

        if (source_files_set.contains(source_file) && target_files_set.contains(info->mimeDataUrl())) {
            has_restore_files << info->fileUrl();
        }
    }

    if (has_restore_files.isEmpty()) {
        return list;
    }

    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRestoreFromTrashEvent>(nullptr, has_restore_files), true);

    return list;
}

static DUrlList pasteFilesV1(const QSharedPointer<DFMPasteEvent> &event)
{
    const DUrlList &urlList = event->urlList();

    if (urlList.isEmpty()) {
        return DUrlList();
    }

    DUrlList list;
    QDir dir(event->targetUrl().toLocalFile());
    //Make sure the target directory exists.
    if (!dir.exists()) {
        return list;
    }

    if (!QFileInfo(event->targetUrl().toLocalFile()).isWritable()) {
        qDebug() << event->targetUrl() << "is not writable";
        DUrlList urls;
        urls << event->targetUrl();

        DFMUrlListBaseEvent noPermissionEvent{event->sender(), urls};
        noPermissionEvent.setWindowId(event->windowId());

        emit fileSignalManager->requestShowNoPermissionDialog(noPermissionEvent);
        return list;
    }

    if (event->action() == DFMGlobal::CutAction) {
        DUrl parentUrl = DUrl::parentUrl(urlList.first());

        if (parentUrl != event->targetUrl()) {
            FileJob job(FileJob::Move);
            job.setWindowId(event->windowId());
            dialogManager->addJob(&job);

            list = job.doMove(urlList, event->targetUrl());
            dialogManager->removeJob(job.getJobId());
        }

        DFMGlobal::clearClipboard();
    } else {

        FileJob job(FileJob::Copy);
        job.setWindowId(event->windowId());
        dialogManager->addJob(&job);

        list = job.doCopy(urlList, event->targetUrl());
        dialogManager->removeJob(job.getJobId());
    }

    return list;
}

DUrlList FileController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    bool use_old_filejob = false;

#ifdef SW_LABEL
    use_old_filejob = true;
#endif

    const DUrlList &urlList = event->urlList();
    DUrlList list;

    if (use_old_filejob) {
        list = pasteFilesV1(event);
    } else {
        list = pasteFilesV2(event->action(), urlList, event->targetUrl());
    }

    DUrlList valid_files = list;

    valid_files.removeAll(DUrl());

    if (valid_files.isEmpty()) {
        return list;
    }

    if (event->action() == DFMGlobal::CopyAction) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, valid_files, true), true);
    } else {
        const QString targetDir(QFileInfo(urlList.first().toLocalFile()).absolutePath());

        if (targetDir.isEmpty()) {
            return list;
        }

        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromLocalFile(targetDir), valid_files), true);
    }

    return list;
}

bool FileController::mkdir(const QSharedPointer<DFMMkdirEvent> &event) const
{
    //Todo:: check if mkdir is ok
    AppController::selectionAndRenameFile = qMakePair(event->url(), event->windowId());

    bool ok = QDir::current().mkdir(event->url().toLocalFile());

    if (ok) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));
    }

    return ok;
}

bool FileController::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    //Todo:: check if mkdir is ok
    QFile file(event->url().toLocalFile());

    AppController::selectionAndRenameFile = qMakePair(event->url(), event->windowId());

    if (file.open(QIODevice::WriteOnly)) {
        file.close();
    } else {
        return false;
    }

    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));

    return true;
}

bool FileController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    QFile file(event->url().toLocalFile());

    event->accept();

    return file.setPermissions(event->permissions());
}

bool FileController::shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const
{
    ShareInfo info;
    info.setPath(event->url().toLocalFile());

    info.setShareName(event->name());
    info.setIsGuestOk(event->allowGuest());
    info.setIsWritable(event->isWritable());

    bool ret = userShareManager->addUserShare(info);

    return ret;
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

bool FileController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    DUrl destUrl = event->url();

    const DAbstractFileInfoPointer &p = fileService->createFileInfo(nullptr, destUrl);
    DUrl bookmarkUrl = DUrl::fromBookMarkFile(destUrl, p->fileDisplayName());
    DStorageInfo info(destUrl.path());
    QString filePath = destUrl.path();
    QString rootPath = info.rootPath();
    if (rootPath != QStringLiteral("/") || rootPath != QStringLiteral("/home")) {
        QString devStr = info.device();
        QString locateUrl;
        int endPos = filePath.indexOf(rootPath);
        if (endPos != -1) {
            endPos += rootPath.length();
            locateUrl = filePath.mid(endPos);
        }
        if (devStr.startsWith(QStringLiteral("/dev/"))) {
            devStr = DUrl::fromDeviceId(info.device()).toString();
        }

        QUrlQuery query;
        query.addQueryItem("mount_point", devStr);
        query.addQueryItem("locate_url", locateUrl);
        bookmarkUrl.setQuery(query);
    }

    return DFileService::instance()->touchFile(event->sender(), bookmarkUrl);
}

bool FileController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->deleteFiles(nullptr, {DUrl::fromBookMarkFile(event->url(), QString())}, false);
}

bool FileController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    QFile file(event->fileUrl().toLocalFile());

    bool ok = file.link(event->toUrl().toLocalFile());

    if (ok) {
        return true;
    }

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

DFileDevice *FileController::createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    if (FileUtils::isGvfsMountFile(event->fileUrl().toLocalFile())) {
        return new DGIOFileDevice(event->fileUrl());
    }

    DLocalFileDevice *device = new DLocalFileDevice();

    device->setFileUrl(event->fileUrl());

    return device;
}

DFileHandler *FileController::createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    Q_UNUSED(event)

    return new DLocalFileHandler();
}

class Match
{
public:
    Match(const QString &group)
    {
        for (const QString &key : DFMApplication::genericObtuselySetting()->keys(group)) {
            const QString &value = DFMApplication::genericObtuselySetting()->value(group, key).toString();

            int last_dir_split = value.lastIndexOf(QDir::separator());

            if (last_dir_split >= 0) {
                QString path = value.left(last_dir_split);

                if (path.startsWith("~/")) {
                    path.replace(0, 1, QDir::homePath());
                }

                patternList << qMakePair(path, value.mid(last_dir_split + 1));
            } else {
                patternList << qMakePair(QString(), value);
            }
        }
    }

    bool match(const QString &path, const QString &name)
    {
        for (auto pattern : patternList) {
            QRegularExpression re(QString(), QRegularExpression::MultilineOption);

            if (!pattern.first.isEmpty()) {
                re.setPattern(pattern.first);

                if (!re.isValid()) {
                    qWarning() << re.errorString();
                    continue;
                }

                if (!re.match(path).hasMatch()) {
                    continue;
                }
            }

            if (pattern.second.isEmpty()) {
                return true;
            }

            re.setPattern(pattern.second);

            if (!re.isValid()) {
                qWarning() << re.errorString();
                continue;
            }

            if (re.match(name).hasMatch()) {
                return true;
            }
        }

        return false;
    }

    QList<QPair<QString, QString>> patternList;
};

bool FileController::customHiddenFileMatch(const QString &absolutePath, const QString &fileName)
{
    static Match match("HiddenFiles");

    return match.match(absolutePath, fileName);
}

bool FileController::privateFileMatch(const QString &absolutePath, const QString &fileName)
{
    static Match match("PrivateFiles");

    return match.match(absolutePath, fileName);
}

bool FileController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    if (event->tags().isEmpty()) {
        const QStringList &tags = TagManager::instance()->getTagsThroughFiles({event->url()});

        return tags.isEmpty() || TagManager::instance()->removeTagsOfFiles(tags, {event->url()});
    }

    return TagManager::instance()->makeFilesTags(event->tags(), {event->url()});
}

bool FileController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    return TagManager::instance()->removeTagsOfFiles(event->tags(), {event->url()});
}

QList<QString> FileController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    return TagManager::instance()->getTagsThroughFiles(event->urlList());
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
    , filters(filter)
{
    bool sort_inode = flags.testFlag(static_cast<QDirIterator::IteratorFlag>(DDirIterator::SortINode));

    if (sort_inode) {
        iterator = new DFMSortInodeDirIterator(path);
    } else {
        iterator = new DFMQDirIterator(path, nameFilters, filter, flags);
    }
}

FileDirIterator::~FileDirIterator()
{
    if (iterator) {
        delete iterator;
    }
}

DUrl FileDirIterator::next()
{
    if (nextIsCached) {
        nextIsCached = false;

        return iterator->fileUrl();
    }

    return iterator->next();
}

bool FileDirIterator::hasNext() const
{
    if (nextIsCached) {
        return true;
    }

    bool hasNext = iterator->hasNext();

    if (!hasNext) {
        return false;
    }

    bool showHidden = filters.testFlag(QDir::Hidden);
    DAbstractFileInfoPointer info;

    do {
        const_cast<FileDirIterator *>(this)->iterator->next();
        info = iterator->fileInfo();

        if (!info->isPrivate() && (showHidden || !info->isHidden())) {
            break;
        }

        info.reset();
    } while (iterator->hasNext());

    // file is exists
    if (info) {
        const_cast<FileDirIterator *>(this)->nextIsCached = true;

        return true;
    }

    return false;
}

QString FileDirIterator::fileName() const
{
    return iterator->fileName();
}

DUrl FileDirIterator::fileUrl() const
{
    return iterator->fileUrl();
}

const DAbstractFileInfoPointer FileDirIterator::fileInfo() const
{
    return iterator->fileInfo();
}

DUrl FileDirIterator::url() const
{
    return iterator->url();
}

bool FileDirIterator::enableIteratorByKeyword(const QString &keyword)
{
#ifdef DISABLE_QUICK_SEARCH
    Q_UNUSED(keyword);
    return false;
#else // !DISABLE_QUICK_SEARCH
    const QString pathForSearching = iterator->url().toLocalFile();

    if (QFileInfo::exists(pathForSearching)) {
        QPair<QString, QString> dev_and_mount_point{ DQuickSearch::getDevAndMountPoint(pathForSearching) };
        bool is_usb_dev{ DQuickSearch::isUsbDevice(dev_and_mount_point.first) };
        bool whether_index_internal{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_IndexInternal).toBool() };
        bool whether_index_external{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_IndexExternal).toBool() };

#ifdef QT_DEBUG
        qDebug() << pathForSearching;
#endif //QT_DEBUG

        std::function<bool()> invoke_quick_search{
            [this, &keyword, &pathForSearching]()->bool
            {
                bool whether_cached{ QuickSearchDaemonController::instance()->createCache() };

#ifdef QT_DEBUG
                qDebug() << whether_cached;
#endif //QT_DEBUG

                if (whether_cached)
                {
                    if (iterator) {
                        delete iterator;
                    }

                    iterator = new DFMQuickSearchDirIterator(pathForSearching, keyword);

#ifdef QT_DEBUG
                    qDebug() << pathForSearching;
#endif //QT_DEBUG
                    return true;
                }

                return false;
            }
        };

        bool whether_use_quick_search{ false };
        if (whether_index_external && whether_index_internal) {
            whether_use_quick_search = invoke_quick_search();

        } else if (!is_usb_dev && whether_index_internal) {
            whether_use_quick_search = invoke_quick_search();

        } else if (is_usb_dev && whether_index_external) {
            whether_use_quick_search = invoke_quick_search();
        }

        return whether_use_quick_search;
    }

    return false;
#endif // DISABLE_QUICK_SEARCH
}
