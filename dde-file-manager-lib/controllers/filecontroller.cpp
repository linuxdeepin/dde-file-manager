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
#include "models/desktopfileinfo.h"
#include "models/trashfileinfo.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "tag/tagmanager.h"

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
#include <QRegularExpression>
#include <QSettings>

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
    QDir::Filters filters;
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

    if (FileUtils::isFileWindowsUrlShortcut(fileUrl.toLocalFile())){
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

    if (oldfilePointer->isDesktopFile() && !oldfilePointer->isSymLink()){
        QString filePath = oldUrl.toLocalFile();
        Properties desktop(filePath, "Desktop Entry");
        QString key;
        QString localKey = QString("Name[%1]").arg(QLocale::system().name());
        if (desktop.contains(localKey)){
            key = localKey;
        }else{
            key = "Name";
        }

        const QString old_name = desktop.value(key).toString();

        desktop.set(key, newfilePointer->fileName());
        result = desktop.save(filePath, "Desktop Entry");

        if (result) {
            const QString path = QFileInfo(file).absoluteDir().absoluteFilePath(old_name);

            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRenameEvent>(nullptr, oldUrl, DUrl::fromLocalFile(path)));
        }
    }else{
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

/**
 * @brief FileController::deleteFiles
 * @param urlList accepted
 *
 * Permanently delete file or directory with the given url.
 */
bool FileController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    FileJob job(FileJob::Delete);
    job.setWindowId(event->windowId());
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
    job.setWindowId(event->windowId());
    dialogManager->addJob(&job);

    DUrlList list = job.doMoveToTrash(event->urlList());
    dialogManager->removeJob(job.getJobId());

    // save event
    const QVariant &result = DFMEventDispatcher::instance()->processEvent<DFMGetChildrensEvent>(event->sender(), DUrl::fromTrashFile("/"),
                                                                                                QStringList(), QDir::AllEntries);
    const QList<DAbstractFileInfoPointer> &infos = qvariant_cast<QList<DAbstractFileInfoPointer>>(result);

    if (infos.isEmpty())
        return list;

    const QSet<DUrl> &source_files_set = event->urlList().toSet();
    const QSet<DUrl> &target_files_set = list.toSet();
    DUrlList has_restore_files;

    for (const DAbstractFileInfoPointer &info : infos) {
        const DUrl &source_file = DUrl::fromLocalFile(static_cast<const TrashFileInfo*>(info.constData())->sourceFilePath());

        if (source_files_set.contains(source_file) && target_files_set.contains(info->mimeDataUrl())) {
            has_restore_files << info->fileUrl();
        }
    }

    if (has_restore_files.isEmpty())
        return list;

    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRestoreFromTrashEvent>(nullptr, has_restore_files), true);

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

    if (!QFileInfo(event->targetUrl().toLocalFile()).isWritable()){
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

    DUrlList valid_files = list;

    valid_files.removeAll(DUrl());

    if (valid_files.isEmpty())
        return list;

    if (event->action() == DFMGlobal::CopyAction) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, valid_files, true), true);
    } else {
        const QString targetDir(QFileInfo(urlList.first().toLocalFile()).absolutePath());

        if (targetDir.isEmpty())
            return list;

        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromLocalFile(targetDir), valid_files), true);
    }

    return list;
}

bool FileController::mkdir(const QSharedPointer<DFMMkdirEvent> &event) const
{
    //Todo:: check if mkdir is ok
    AppController::selectionAndRenameFile = qMakePair(event->url(), event->windowId());

    bool ok = QDir::current().mkdir(event->url().toLocalFile());

    if (ok)
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));

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

    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event , dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));

    return true;
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

class Match
{
public:
    Match(const QString &group)
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(),
                           "dde-file-manager/" + qApp->applicationName());

        settings.setIniCodec("utf-8");
        settings.beginGroup(group);

        for (const QString &key : settings.childKeys()) {
            const QString &value = settings.value(key).toString();

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

        settings.endGroup();
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

                if (!re.match(path).hasMatch())
                    continue;
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
    static Match match("Hidden Files");

    return match.match(absolutePath, fileName);
}

bool FileController::privateFileMatch(const QString &absolutePath, const QString &fileName)
{
    static Match match("Private Files");

    return match.match(absolutePath, fileName);
}

bool FileController::makeFileTags(const QSharedPointer<DFMMakeFileTagsEvent> &event) const
{
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
    , iterator(path, nameFilters, filter, flags)
    , filters(filter)
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
    if (!processRlocate) {
        if (currentFileInfo.exists() || currentFileInfo.isSymLink()) {
            DUrl url = DUrl::fromLocalFile(currentFileInfo.absoluteFilePath());

            currentFileInfo.setFile(QString());

            return url;
        }

        return DUrl::fromLocalFile(iterator.next());
    }

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
    if (!processRlocate) {
        if (currentFileInfo.exists() || currentFileInfo.isSymLink())
            return true;

        bool hasNext = iterator.hasNext();
        bool showHidden = filters.testFlag(QDir::Hidden);

        if (!hasNext)
            return false;

        DFileInfo *info = nullptr;

        while (iterator.hasNext()) {
            const_cast<FileDirIterator*>(this)->iterator.next();
            info = new DFileInfo(iterator.fileInfo(), false);

            if (!info->isPrivate() && (showHidden || !info->isHidden())) {
                break;
            }

            delete info;
            info = nullptr;
        }

        // file is exists
        if (info) {
            const_cast<FileDirIterator*>(this)->currentFileInfo = info->toQFileInfo();
            delete info;

            return true;
        }

        return false;
    }

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
