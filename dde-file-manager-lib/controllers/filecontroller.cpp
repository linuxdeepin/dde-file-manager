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
#include "shutil/dfmregularexpression.h"
#include "shutil/dfmfilelistfile.h"

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
#include <QMutex>

#include "dfmsettings.h"
#include "dfmapplication.h"
#ifndef DISABLE_QUICK_SEARCH
#include "anything_interface.h"
#endif

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

        if (!info.isSymLink() && FileUtils::isDesktopFile(info)) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(info));
        }

        return DAbstractFileInfoPointer(new DFileInfo(info));
    }
    //判读ios手机，传输慢，需要特殊处理优化
    const DAbstractFileInfoPointer optimiseFileInfo() const override
    {
        const QFileInfo &info = iterator.fileInfo();
        DUrl url = DUrl::fromLocalFile(info.absoluteFilePath());
        url.setOptimise(true);
        QString abfilepath = info.absoluteFilePath();
        if (info.isDir())
            abfilepath += QString("/");
        if (!info.isSymLink() && FileUtils::isDesktopFileOptmise(abfilepath)) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(url));
        }

        return DAbstractFileInfoPointer(new DFileInfo(url));
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

#ifndef DISABLE_QUICK_SEARCH
class DFMAnythingDirIterator : public DDirIterator
{
public:
    DFMAnythingDirIterator(ComDeepinAnythingInterface *u,
                           const QString &path, const QString &k)
        : interface(u)
            , keyword(k)
            , dir(path)
    {
        keyword = DFMRegularExpression::checkWildcardAndToRegularExpression(keyword);
    }

    ~DFMAnythingDirIterator() override
    {

    }

    DUrl next() override
    {
        currentFileInfo.setFile(searchResults.takeFirst());

        return fileUrl();
    }

    bool hasNext() const override
    {
        if (!initialized) {
            const QString &dir_path = dir.absolutePath();
            // 如果挂载在此路径下的其它目录也支持索引数据, 则一并搜索
            searchDirList << interface->hasLFTSubdirectories(dir_path);

            if (searchDirList.isEmpty() || searchDirList.first() != dir_path) {
                searchDirList.prepend(dir_path);
            }

            initialized = true;
        }

        while (searchResults.isEmpty() && !searchDirList.isEmpty()) {
            const auto result = interface->search(100, 500, searchStartOffset, searchEndOffset, searchDirList.first(), keyword, true);

            searchResults = result.argumentAt<0>();
            searchStartOffset = result.argumentAt<1>();
            searchEndOffset = result.argumentAt<2>();

            // 当前目录已经搜索到了结尾
            if (searchStartOffset >= searchEndOffset) {
                searchStartOffset = searchEndOffset = 0;
                searchDirList.removeAt(0);
            }
        }

        return !searchResults.isEmpty();
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
    ComDeepinAnythingInterface *interface;
    QString keyword;

    mutable bool initialized = false;
    mutable QStringList searchDirList;
    mutable quint32 searchStartOffset = 0, searchEndOffset = 0;
    mutable QStringList searchResults;

    QDir dir;
    QFileInfo currentFileInfo;
};
#endif // DISABLE_QUICK_SEARCH

class FileDirIterator : public DDirIterator
{
public:
    FileDirIterator(const QString &url,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    ~FileDirIterator() override;

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    //判读ios手机，传输慢，需要特殊处理优化
    const DAbstractFileInfoPointer optimiseFileInfo() const Q_DECL_OVERRIDE;
    DUrl url() const Q_DECL_OVERRIDE;

    bool enableIteratorByKeyword(const QString &keyword) Q_DECL_OVERRIDE;

    DFMFileListFile *hiddenFiles = nullptr;

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

const DAbstractFileInfoPointer FileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    DUrl url = event->url();
    QString localFile = url.toLocalFile();
    QFileInfo info(localFile);
    //处理苹果文件是否优化
    bool boptimise = url.isOptimise();
    if (boptimise) {
        QString abfilepath = info.absoluteFilePath();
        if (info.isDir())
            abfilepath += QString("/");
        if (!info.isSymLink() && FileUtils::isDesktopFileOptmise(abfilepath)) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(event->url()));
        }
    } else {
        if (!info.isSymLink() && FileUtils::isDesktopFile(localFile)) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(event->url()));
        }
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

    const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

        if (linkInfo && !linkInfo->exists()) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
            return false;
        }
        if (!linkInfo) {
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

    if (FileUtils::shouldAskUserToAddExecutableFlag(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
        int code = dialogManager->showAskIfAddExcutableFlagAndRunDialog(fileUrl, event->windowId());
        return FileUtils::addExecutableFlagAndExecuse(fileUrl.toLocalFile(), code);
    }

    QString url = fileUrl.toLocalFile();
    if (FileUtils::isFileWindowsUrlShortcut(url)) {
        url = FileUtils::getInternetShortcutUrl(url);
    }

    bool result = FileUtils::openFile(url);
    if (!result) {
        AppController::instance()->actionOpenWithCustom(event); // requestShowOpenWithDialog
    }

    return result;
}

bool FileController::openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const
{
    DUrlList fileUrls = event->urlList();
    DUrlList packUrl;
    QStringList pathList;
    bool result = false;

    for (DUrl fileUrl : fileUrls) {
        const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

        if (pfile->isSymLink()) {
            const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

            if (linkInfo && !linkInfo->exists()) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
        }

        if (FileUtils::isExecutableScript(fileUrl.toLocalFile())) {
            int code = dialogManager->showRunExcutableScriptDialog(fileUrl, event->windowId());
            result = FileUtils::openExcutableScriptFile(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        if (FileUtils::isFileRunnable(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
            int code = dialogManager->showRunExcutableFileDialog(fileUrl, event->windowId());
            result = FileUtils::openExcutableFile(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        if (FileUtils::shouldAskUserToAddExecutableFlag(fileUrl.toLocalFile()) && !pfile->isDesktopFile()) {
            int code = dialogManager->showAskIfAddExcutableFlagAndRunDialog(fileUrl, event->windowId());
            result = FileUtils::addExecutableFlagAndExecuse(fileUrl.toLocalFile(), code) || result;
            continue;
        }

        packUrl << fileUrl;
        QString url = fileUrl.toLocalFile();
        if (FileUtils::isFileWindowsUrlShortcut(url)) {
            url = FileUtils::getInternetShortcutUrl(url);
        }
        pathList << url;
    }

    if (!pathList.empty()) {
        result = FileUtils::openFiles(pathList);
        if (!result) {
            for (const DUrl &fileUrl : packUrl) {
                AppController::instance()->actionOpenWithCustom(dMakeEventPointer<DFMOpenFileEvent>(event->sender(), fileUrl)); // requestShowOpenWithDialog
            }
        }
    }

    return result;
}

bool FileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    //处理快捷方式，还原成原路径
    DUrl fileUrl = event->url();

    const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

        if (linkInfo && !linkInfo->exists()) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
            return false;
        }
        const_cast<DUrl &>(fileUrl) = linkInfo->redirectedFileUrl();
    }
    return FileUtils::openFilesByApp(event->appName(), {fileUrl.toString()});
}

bool FileController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
{
    //处理快捷方式，还原成原路径
    QList<DUrl> fileUrls = event->urlList();

    QStringList pathList;

    for (DUrl fileUrl : fileUrls) {
        const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

        if (pfile->isSymLink()) {
            const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

            if (linkInfo && !linkInfo->exists()) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
        }
        QString url = fileUrl.toLocalFile();
        if (FileUtils::isFileWindowsUrlShortcut(url)) {
            url = FileUtils::getInternetShortcutUrl(url);
        }
        pathList << url;
    }

    return FileUtils::openFilesByApp(event->appName(), pathList);

}

bool FileController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
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
        dialogManager->showErrorDialog(tr("Kindly Reminder"), tr("Please install File Roller first and then continue"));
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
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
        dialogManager->showErrorDialog(tr("Kindly Reminder"), tr("Please install File Roller first and then continue"));
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const
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
        dialogManager->showErrorDialog(tr("Kindly Reminder"), tr("Please install File Roller first and then continue"));
        qDebug() << "file-roller is not installed";
    }

    return false;
}

bool FileController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    //计算机和回收站桌面文件不能被复制或剪切，从这里过滤通过快捷键复制剪切的计算机和回收站桌面文件url
    DUrlList urlList;
    for (const DUrl &url : event->urlList()) {
        if ((DesktopFileInfo::computerDesktopFileUrl() == url) ||
                (DesktopFileInfo::trashDesktopFileUrl() == url) ||
                (DesktopFileInfo::homeDesktopFileUrl() == url)) {
            continue;
        }

        urlList.append(url);
    }

    if (urlList.isEmpty()) {
        return false;
    }

    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(urlList), event->action());

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
        desktop.set("X-Deepin-Vendor", QStringLiteral("user-custom"));
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

        // mtp 目录无法删除，因此采用复制再删除的模式
        if (!result && QFileInfo(file).isDir()) {
            result = QProcess::execute("cp -r \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;

            if (result) {
                QProcess::execute("rm -r \"" + file.fileName().toUtf8() + "\"");
            } else {
                QProcess::execute("rm -r \"" + newFilePath.toUtf8() + "\"");
            }
        }

        if (result) {
            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRenameEvent>(nullptr, newUrl, oldUrl));

            //重命名成功时需要判断该文件原路径是否被添加到了剪贴版，如果是，就需要替换剪贴板路径。
            QList<QUrl> clipUrls = DFMGlobal::fetchUrlsFromClipboard();
            bool needReset = false;
            for (QUrl &clipUrl : clipUrls) {
                if (clipUrl.path() == oldUrl.path()) {
                    clipUrl.setUrl(newUrl.url());
                    needReset = true;
                }
            }

            if (needReset) {
                DFMGlobal::setUrlsToClipboard(clipUrls, DFMGlobal::fetchClipboardAction());
            }
        }
    }

    return result;
}

bool FileController::isExtDeviceJobCase(void *curJob, const DUrl &url) const
{
    DFileCopyMoveJob *thisJob = (DFileCopyMoveJob *)curJob;
    QString filePath = url.path();
    DUrlList srcUrlList = thisJob->sourceUrlList();
    DUrl targetUrl = thisJob->targetUrl();

    bool isDiscCase = false; // 查看是否是一般case 路径，比如FAT32 U盘直接写数据
    if (targetUrl.path().contains(filePath)) {
        isDiscCase = true;
    }

    foreach (DUrl oneUrl, srcUrlList) {
        if (oneUrl.path().contains(filePath)) {
            isDiscCase = true;
            break;
        }
    }

    if (isDiscCase)
        return true;

    return  isDiscburnJobCase(curJob, url);
}

bool FileController::isDiscburnJobCase(void *curJob, const DUrl &url) const
{
    DFileCopyMoveJob *thisJob = (DFileCopyMoveJob *)curJob;

    QString burnDestDevice = url.burnDestDevice();

    DUrlList srcUrlList = thisJob->sourceUrlList();
    DUrl targetUrl = thisJob->targetUrl();

    burnDestDevice.replace('/', '_');

    // 查看当前路径是否是光驱缓存路径
    bool isDiscCase = false;
    if (targetUrl.path().contains(DISCBURN_CACHE_MID_PATH) &&
            targetUrl.path().contains(burnDestDevice)) {
        isDiscCase = true;
    }

    foreach (DUrl oneUrl, srcUrlList) {
        if (oneUrl.path().contains(DISCBURN_CACHE_MID_PATH) &&
                oneUrl.path().contains(burnDestDevice)) {
            isDiscCase = true;
            break;
        }
    }

    return  isDiscCase;
}
// fix bug 35855修改复制拷贝流程，拷贝线程不去阻塞主线程，拷贝线程自己去处理，主线程直接返回，拷贝线程结束了在去处理以前的后续操作，delete还是走老流程
DUrlList FileController::pasteFilesV2(const QSharedPointer<DFMPasteEvent> &event, DFMGlobal::ClipboardAction action, const DUrlList &list, const DUrl &target, bool slient, bool force, bool bold) const
{
    // fix bug 27109 在某种情况下，存在 FileCopyMoveJob 还没被析构，但其中的成员 StatisticJob 已经被析构，又在 FileCopyMoveJob 的函数中调用了 StatisticJob 的对象，导致崩溃
    // 所以这里将原来的普通指针以 deleteLater 析构的内存管理方式交给智能指针去判定。测试百次左右没有再发生崩溃的现象。
    // 该现象发生于从搜索列表中往光驱中发送文件夹还不被支持的时候。现已可以从搜索列表、最近列表、标签列表中往光驱中发送文件
    QSharedPointer<DFileCopyMoveJob> job = QSharedPointer<DFileCopyMoveJob>(new DFileCopyMoveJob());
    // 当前线程退出，局不变currentJob被释放，但是ErrorHandle线程还在使用它
    // fix bug 31324,判断当前操作是否是清空回收站，是就在结束时改变清空回收站状态
    bool bdoingcleartrash = DFileService::instance()->getDoClearTrashState();
    if (action == DFMGlobal::CutAction && bdoingcleartrash && list.count() == 1 &&
            list.first().toString().endsWith(".local/share/Trash/files")) {
        connect(job.data(), &QThread::finished, this, [ = ]() {
            DFileService::instance()->setDoClearTrashState(false);
        });
    }

    if (force) {
        job->setFileHints(DFileCopyMoveJob::ForceDeleteFile);
    }

    // sp3 feature： 复制时不进行校验，后面调整为独立的功能
    job->setFileHints(job->fileHints() | DFileCopyMoveJob::DontIntegrityChecking);
    if (action == DFMGlobal::CutAction && !target.isValid()) {
        // for remove mode
        job->setActionOfErrorType(DFileCopyMoveJob::NonexistenceError, DFileCopyMoveJob::SkipAction);
    }

    if (QThread::currentThread()->loopLevel() <= 0) {
        // 确保对象所在线程有事件循环
        job->moveToThread(qApp->thread());
    }

    class ErrorHandle : public QObject, public DFileCopyMoveJob::Handle
    {
    public:
        ErrorHandle(QSharedPointer<DFileCopyMoveJob> job, bool s)
            : QObject(nullptr)
            , slient(s)
            , fileJob(job)
        {
            //线程启动传递源地址和目标地址
            connect(job.data(), &DFileCopyMoveJob::currentJobChanged, this, [this](const DUrl & from, const DUrl & to) {
                QMutex mutex;
                mutex.lock();
                currentJob.first = from;
                currentJob.second = to;
                mutex.unlock();
            }, Qt::DirectConnection);
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
            qDebug() << " ErrorHandle() ";
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

            DFileCopyMoveJob::Handle *handle = dialogManager->taskDialog()->addTaskJob(job);
            emit job->currentJobChanged(currentJob.first, currentJob.second);

            if (!handle) {
                qWarning() << "addTaskJob create handle failed!!";
                return DFileCopyMoveJob::SkipAction;
            }

            return handle->handleError(job, error, sourceInfo, targetInfo);
        }

        void timerEvent(QTimerEvent *e) override
        {
            if (e->timerId() != timer_id) {
                return QObject::timerEvent(e);
            }

            killTimer(timer_id);
            timer_id = 0;

            //1. 此时说明pasteFilesV2函数已经结束
            if (!fileJob)
                return;
            //这里会出现pasteFilesV2函数线程和当前线程是同时在执行，会出现在1处pasteFilesV2所在线程 没结束，但是这时pasteFilesV2所在线程 结束
            //这里是延时处理，会出现正在执行吃此处代码时，filejob线程完成了
            if (!fileJob->isFinished()) {
                dialogManager->taskDialog()->addTaskJob(fileJob.data());
                emit fileJob->currentJobChanged(currentJob.first, currentJob.second);
            }
        }

        int timer_id = 0;
        bool slient;
        QSharedPointer<DFileCopyMoveJob> fileJob;
        QPair<DUrl, DUrl> currentJob;
    };

    ErrorHandle *error_handle = new ErrorHandle(job, slient);

    // bug 29419 期望在外设进行卸载，弹出时，终止复制操作
    DFileCopyMoveJob *thisJob = job.data();
    connect(fileSignalManager, &FileSignalManager::requestAsynAbortJob, thisJob, [thisJob, this](const DUrl & url) {

        bool isExtDeviceWorkingJob = isExtDeviceJobCase(thisJob, url);
        if (isExtDeviceWorkingJob) {

            emit thisJob->stop();
            qDebug() << "break the FileCopyMoveJob for the device:" << url.path();

            thisJob->wait(); // 等job线程结束
            sleep(1); // 加一个buffer 时间等外设相关设备结束
        }

    });

    job->setErrorHandle(error_handle, slient ? nullptr : error_handle->thread());
    // fix：原来本来没有 movemode，当前彻底删除文件时要转换 mode 为 movemode，
    //     原来剪切和删除都是用的 cutmode 来表示，这会导致进度条无法区分剪切和删除
    //     为了区分剪切和删除，这里使用 traget 是否为空来判断当前的操作是剪切还是删除
    if (target.isEmpty())
        action = DFMGlobal::UnknowAction;
    job->setMode(action == DFMGlobal::CopyAction
                 ? DFileCopyMoveJob::CopyMode
                 : (action == DFMGlobal::CutAction ? DFileCopyMoveJob::CutMode : DFileCopyMoveJob::MoveMode));
    job->start(list, target);
    //走以前的老流程，阻塞主线去拷贝或者删除
    if (bold) {
        job->wait();

        QTimer::singleShot(200, dialogManager->taskDialog(), [job] {
            dialogManager->taskDialog()->removeTaskJob(job.data());
        });
        //当前线程不要去处理error_handle所在的线程资源
        //    error_handle->currentJob = nullptr;
        //    error_handle->fileJob = nullptr;

        if (slient) {
            error_handle->deleteLater();
        } else {
            QMetaObject::invokeMethod(error_handle, "deleteLater");
        }

        //    QMetaObject::invokeMethod(job, "deleteLater");

        return job->targetUrlList();
    }
    //fix bug 35855走新流程不去阻塞主线程，拷贝线程自己去运行，主线程返回，当拷贝线程结束了再去处理以前的相应处理
    connect(job.data(), &QThread::finished, dialogManager->taskDialog(), [this, job, error_handle, slient, event] {
        dialogManager->taskDialog()->removeTaskJob(job.data());
        if (slient)
        {
            error_handle->deleteLater();
        } else
        {
            QMetaObject::invokeMethod(error_handle, "deleteLater");
        }
        //处理复制、粘贴和剪切(拷贝)结束后操作 fix bug 35855
        this->dealpasteEnd(job->targetUrlList(), event);
    });

    return job->targetUrlList();
}

void FileController::dealpasteEnd(const DUrlList &list, const QSharedPointer<DFMPasteEvent> &event) const
{
    DUrlList valid_files = list;

    valid_files.removeAll(DUrl());

    if (valid_files.isEmpty()) {
        //到dfileservice里面作处理
        DFileService::instance()->dealPasteEnd(event, list);
        return;
    }

    if (event->action() == DFMGlobal::CopyAction) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, valid_files, true), true);
    } else {
        const QString targetDir(QFileInfo(event->urlList().first().toLocalFile()).absolutePath());

        if (targetDir.isEmpty()) {
            //到dfileservice里面作处理
            DFileService::instance()->dealPasteEnd(event, list);
            return;
        }

        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromLocalFile(targetDir), valid_files), true);
    }

    //到dfileservice里面作处理
    DFileService::instance()->dealPasteEnd(event, list);
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
    // 解决撤销操作后文件删除不提示问题
    //    if (event->type() == DFMEvent::DeleteFiles) {
    //        return DFileService::instance()->deleteFiles(nullptr, event->urlList(), false);
    //    }


    bool ok = !pasteFilesV2(nullptr, DFMGlobal::CutAction, event->fileUrlList(), DUrl(), event->silent(), event->force(), true).isEmpty();
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
    //以前的老代码
#if 0
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
#endif

    //新代码,修改复制拷贝流程，拷贝线程不去阻塞主线程，拷贝线程自己去处理，主线程直接返回，拷贝线程结束了在去处理以前的后续操作

    bool use_old_filejob = false;

#ifdef SW_LABEL
    use_old_filejob = true;
#endif

    DUrlList list;
    //pasteFilesV1走以前的流程
    if (use_old_filejob) {
        list = pasteFilesV1(event);

        DUrlList valid_files = list;

        valid_files.removeAll(DUrl());

        if (valid_files.isEmpty()) {
            //到dfileservice里面作处理
            DFileService::instance()->dealPasteEnd(event, list);
            return list;
        }

        if (event->action() == DFMGlobal::CopyAction) {
            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, valid_files, true), true);
        } else {
            const QString targetDir(QFileInfo(event->fileUrlList().first().toLocalFile()).absolutePath());

            if (targetDir.isEmpty()) {
                //到dfileservice里面作处理
                DFileService::instance()->dealPasteEnd(event, list);
                return list;

            }

            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromLocalFile(targetDir), valid_files), true);
        }
        //到dfileservice里面作处理
        DFileService::instance()->dealPasteEnd(event, list);
    } else {
        list = pasteFilesV2(event, event->action(), event->urlList(), event->targetUrl());
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

bool FileController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
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

    bool ok = QProcess::startDetached(FileUtils::defaultTerminalPath());

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

    if (event->force()) {
        // replace symlink, remove if target was existed
        QFileInfo toLink(event->toUrl().toLocalFile());
        if (toLink.isSymLink() || toLink.exists()) {
            QFile::remove(event->toUrl().toLocalFile());
        }
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

DStorageInfo *FileController::createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    return new DStorageInfo(event->url().toLocalFile());
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

    // misc, not related to the file iterator at all.
    hiddenFiles = new DFMFileListFile(path);
}

FileDirIterator::~FileDirIterator()
{
    if (iterator) {
        delete iterator;
    }

    if (hiddenFiles) {
        delete hiddenFiles;
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
        if (m_boptimise) {
            info = iterator->optimiseFileInfo();
        }
        if (!info) {
            info = iterator->fileInfo();
        }
        //特殊判断苹果.AAEAAE文件是iPhone编辑照片时留下的缓存文件。 它记录了编辑照片时用了什么滤镜,图片裁了多少等等编辑步骤和效果。可以说AAE文件就是一堆操作的记录文件。直接隐藏
        bool bhide = info->fileName().endsWith(QString(".AAE"));
        if (!bhide && !info->isPrivate() && (showHidden || (!info->isHidden() && !hiddenFiles->contains(info->fileName())))) {
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
//判读ios手机，传输慢，需要特殊处理优化
const DAbstractFileInfoPointer FileDirIterator::optimiseFileInfo() const
{
    return iterator->optimiseFileInfo();
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

    static ComDeepinAnythingInterface anything("com.deepin.anything", "/com/deepin/anything",
                                               QDBusConnection::systemBus());

    if (!anything.hasLFT(pathForSearching)) {
        return false;
    } else {
        qDebug() << "support quick search for: " << pathForSearching;
    }

    if (iterator)
        delete iterator;

    iterator = new DFMAnythingDirIterator(&anything, pathForSearching, keyword);

    return true;
#endif // DISABLE_QUICK_SEARCH
}
