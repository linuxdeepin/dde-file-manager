// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecontroller.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfilewatcher.h"
#include "dfileinfo.h"
#include "dgvfsfileinfo.h"
#include "trashmanager.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dlocalfiledevice.h"
#include "dgiofiledevice.h"
#include "dlocalfilehandler.h"
#include "dfilecopymovejob.h"
#include "dstorageinfo.h"
#include <sys/stat.h>
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
#include "interfaces/dfmstandardpaths.h"

#include "appcontroller.h"
#include "singleton.h"

#include "models/sharefileinfo.h"
#include "usershare/usersharemanager.h"

#include "fileoperations/sort.h"

#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "controllers/vaultcontroller.h"
#include "controllers/masteredmediacontroller.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QGuiApplication>
#include <QUrlQuery>
#include <QRegularExpression>

#include <thread>
#include <unistd.h>
#include <gio/gio.h>

#include <QQueue>
#include <QMutex>
#include <QTextCodec>

#include "dfmsettings.h"
#include "dfmapplication.h"

class DFMQDirIterator : public DDirIterator
{
public:
    DFMQDirIterator(const QString &path,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags,
                    const bool isgvfs = false)
        : iterator(path, nameFilters, filter, flags)
        , isgvfs(isgvfs)
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
        bool isnotsyslink = !info.isSymLink();
        //父目录是gvfs目录，那么子目录和子文件必须是gvfs文件
        QString path = info.path();
        bool currentisgvfs = isgvfs ? true : FileUtils::isGvfsMountFile(path, true);
        QMimeType mimetype;
        bool isdesktop = currentisgvfs ? FileUtils::isDesktopFile(info, mimetype) :
                         FileUtils::isDesktopFile(info);
        if (isnotsyslink && isdesktop) {
            return DAbstractFileInfoPointer(new DesktopFileInfo(info));
        }

        if (currentisgvfs) {
            return DAbstractFileInfoPointer(new DGvfsFileInfo(info, mimetype, false));
        }
        return DAbstractFileInfoPointer(new DFileInfo(info));
    }

    DUrl url() const override
    {
        return DUrl::fromLocalFile(iterator.path());
    }

private:
    QDirIterator iterator;
    bool isgvfs = false;
};

class DFMSortInodeDirIterator : public DDirIterator
{
public:
    explicit DFMSortInodeDirIterator(const QString &path)
        : dir(path)
    {

    }

    ~DFMSortInodeDirIterator() override
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
        //判断是否是gvfs文件，是就创建DGvfsFileInfo
        bool currentisgvfs = FileUtils::isGvfsMountFile(currentFileInfo.path(), true);
        if (currentisgvfs) {
            return DAbstractFileInfoPointer(new DGvfsFileInfo(currentFileInfo, false));
        }
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

class FileDirIterator : public DDirIterator
{
public:
    FileDirIterator(const QString &url,
                    const QStringList &nameFilters,
                    QDir::Filters filter,
                    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                    const bool gvfs = false);
    ~FileDirIterator() override;

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;
    void close() override;
    bool enableIteratorByKeyword(const QString &keyword) override;

    DFMFileListFile *hiddenFiles = nullptr;

private:
    DDirIterator *iterator = nullptr;
    QDir::Filters filters;
    bool nextIsCached = false;
    QHash<DUrl, DAbstractFileInfoPointer> nextInofCached;
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

    //父目录是gvfs目录，那么子目录和子文件必须是gvfs文件
    bool currentisgvfs = FileUtils::isGvfsMountFile(url.toLocalFile(), true);

    QMimeType mimetype;
    bool isdesktop = currentisgvfs ? FileUtils::isDesktopFile(localFile, mimetype) :
                     FileUtils::isDesktopFile(localFile);

    if (isdesktop) {
        QFileInfo info(localFile); // time cost is about 100 ms
        if (!info.isSymLink()) {
            return  DAbstractFileInfoPointer(new DesktopFileInfo(event->url()));
        }
    }

    if (currentisgvfs) {
        //TODO: create FileInfo cost about 1000 ms on andriod mobile
        return DAbstractFileInfoPointer(new DGvfsFileInfo(event->url(), false));
    }

    return DAbstractFileInfoPointer(new DFileInfo(event->url()));
}

const DDirIteratorPointer FileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new FileDirIterator(event->url().toLocalFile(), event->nameFilters(),
                                                   event->filters(), event->flags(), event->isGvfsFile()));
}

bool FileController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    DUrl fileUrl = event->url();

    const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());

        if (!linkInfo)  {
            dialogManager->showErrorDialog(tr("Unable to find the original file"), QString());
            return false;
        }
        const_cast<DUrl &>(fileUrl) = linkInfo->redirectedFileUrl();
        if (!linkInfo->exists() && !FileUtils::isSmbUnmountedFile(fileUrl)) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), pfile->fileUrl());
            return false;
        }
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
        QFile file(url);
        if (file.exists()) {//! 文件打开失败，排除已被删除的原因
            AppController::instance()->actionOpenWithCustom(event); // requestShowOpenWithDialog
        }
        file.close();
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

            if (!linkInfo)  {
                dialogManager->showErrorDialog(tr("Unable to find the original file"), QString());
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
            if (!linkInfo->exists() && !FileUtils::isSmbUnmountedFile(fileUrl)) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), pfile->fileUrl());
                continue;
            }
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
        if (event->isEnter()) {
            result = FileUtils::openEnterFiles(pathList);
        } else {
            result = FileUtils::openFiles(pathList);
        }
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

        if (!linkInfo)  {
            dialogManager->showErrorDialog(tr("Unable to find the original file"), QString());
            return false;
        }
        const_cast<DUrl &>(fileUrl) = linkInfo->redirectedFileUrl();
        if (!linkInfo->exists() && !FileUtils::isSmbUnmountedFile(fileUrl)) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), pfile->fileUrl());
            return false;
        }
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

            if (!linkInfo)  {
                dialogManager->showErrorDialog(tr("Unable to find the original file"), QString());
                continue;
            }
            fileUrl = linkInfo->redirectedFileUrl();
            if (!linkInfo->exists() && !FileUtils::isSmbUnmountedFile(fileUrl)) {
                dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), pfile->fileUrl());
                continue;
            } 
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
    DUrlList urlList = event->urlList();
    urlList.removeAll(DesktopFileInfo::computerDesktopFileUrl());
    urlList.removeAll(DesktopFileInfo::trashDesktopFileUrl());
    urlList.removeAll(DesktopFileInfo::homeDesktopFileUrl());

    if (urlList.isEmpty()) {
        return false;
    }

    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(urlList), event->action());

    return true;
}

bool FileController::renameFileByGio(const DUrl &oldUrl, const DUrl &newUrl) const
{
    bool result = false;
    QString fname = oldUrl.fileName();
    QString tname = newUrl.fileName();
    QString from = oldUrl.parentUrl().toLocalFile();
    QString to = newUrl.parentUrl().toLocalFile();

    if (to.compare(from) != 0) {
        qDebug() << "gio API can not rename file or directory those are not under same path!";
        return false;
    }

    //获取当前目录
    const QString &curd = QDir::currentPath();

    GError *error = nullptr;
    if (!QDir::setCurrent(to)) {
        qDebug() << "failed to chdir " << to;
        return false;
    }

    GFile *file = g_file_new_for_path(fname.toStdString().c_str());
    GFile *new_file = g_file_set_display_name(file, tname.toStdString().c_str(), nullptr, &error);
    if (new_file == nullptr) {
        qDebug() << error->message;
        g_error_free(error);
    } else {
        char *path = g_file_get_path(new_file);
        qDebug() << "Rename successful. New path: " << path;
        g_object_unref(new_file);
        g_free(path);
        result = true;
    }

    g_object_unref(file);

    if (result) {
        emit fileSignalManager->fileMoved(from, fname, to, tname);
    }

    if (!QDir::setCurrent(curd)) {
        qDebug() << "failed to return to directory " << curd;
    }

    return result;
}

bool FileController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const DUrl &oldUrl = event->fromUrl();
    const DUrl &newUrl = event->toUrl();

    QFile file(oldUrl.toLocalFile());
    const QString &newFilePath = newUrl.toLocalFile();

    const DAbstractFileInfoPointer &oldfilePointer = DFileService::instance()->createFileInfo(this, oldUrl);
    const DAbstractFileInfoPointer &newfilePointer = DFileService::instance()->createFileInfo(this, newUrl);

    const QString &newFileName = newfilePointer->fileName();
    // do hidden name remind
    if(event->checkHide() && !FileController::doHiddenFileRemind(newFileName))
        return true;

    bool result(false);

    if (oldfilePointer->isDesktopFile() && !oldfilePointer->isSymLink()) {
        QString filePath = oldUrl.toLocalFile();
        Properties desktop(filePath, "Desktop Entry");
        QString key;
        const QString local = QLocale::system().name();
        const QString keyTemplate = QString("Name[%1]");
        QString localKey = keyTemplate.arg(local);
        if (desktop.contains(localKey)) {
            key = localKey;
        } else {
            // split xx_XX
            auto splitedLocal = local.trimmed().split("_");
            if (!splitedLocal.isEmpty()
                    && desktop.contains(localKey = keyTemplate.arg(splitedLocal.first())))
                key = localKey;
             else
                key = "Name";
        }

        const QString currentName = oldfilePointer->fileDisplayName();

        const QString old_name = desktop.value(key).toString();
        // 名称没有改变
        if (currentName == newfilePointer->fileName()) {
            qDebug() << "rename" << filePath << "currentName"
                     << currentName << "key" << key  << old_name << "new name" << newfilePointer->fileName();
            return true;
        }

        desktop.set(key, newfilePointer->fileName());
        desktop.set("X-Deepin-Vendor", QStringLiteral("user-custom"));
        result = desktop.save(filePath, "Desktop Entry");

        if (result) {
            const QString path = QFileInfo(file).absoluteDir().absoluteFilePath(old_name);

            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMRenameEvent>(nullptr, oldUrl, DUrl::fromLocalFile(path)));
        }
    } else {
        if (newFilePath.contains("gvfs/mtp")) {
            result = renameFileByGio(oldUrl, newUrl);
        }

        if (!result) {
            result = file.rename(newFilePath);
        }

        if (!result) {
            result = QProcess::execute("mv \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;
        }

        // mtp ftp(和ftp搭建的配置有关系) 目录无法删除，因此采用复制再删除的模式
        if (!result && (FileUtils::isFtpFile(oldUrl) || newFilePath.contains("gvfs/mtp")) && QFileInfo(file).isDir()) {
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
    DFileCopyMoveJob *thisJob = static_cast<DFileCopyMoveJob *>(curJob);
    if (!thisJob)
        return false;
    DUrlList srcUrlList = thisJob->sourceUrlList();
    DUrl targetUrl = thisJob->targetUrl();

    QString devId;
    QString filePath = url.path(); // 转换光盘路径为实际挂载路径
    if (url.scheme() == BURN_SCHEME) {
        devId = url.path().remove("/" BURN_SEG_ONDISC "/").replace("/", "_"); // /dev/sr0/disc_files/ ——> _dev_sr0
        foreach (auto d, deviceListener->getDeviceList()) {
            if (url.path().contains(d->getId())) {
                filePath = d->getMountPoint();
                filePath.remove("file://");
                break;
            }
        }
    }

    static const QString stagingPathPrefix = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/";
    QString stagingPath;
    if (!devId.isEmpty())
        stagingPath = stagingPathPrefix + devId;
    else
        stagingPath = filePath; // 不为光驱时，目标路径就是设备的挂载点
    // 源路径为光盘文件或目标路径为暂存路径
    if (!stagingPath.isEmpty() && targetUrl.path().contains(stagingPath))
        return true;
    foreach (auto u, srcUrlList) {
        if (u.path().contains(filePath))
            return true;
    }
    return isDiscburnJobCase(thisJob, url);
}

bool FileController::isDiscburnJobCase(void *curJob, const DUrl &url) const
{
    DFileCopyMoveJob *thisJob = static_cast<DFileCopyMoveJob *>(curJob);

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
    //! 当前拷贝如果是退回到回收站，如要在job中保存回收站中文件原路径
    if (!event.isNull() && !qvariant_cast<DUrlList>(event->cutData()).isEmpty()) {
        job->setCurTrashData(event->cutData());
    }

    // 当前线程退出，局不变currentJob被释放，但是ErrorHandle线程还在使用它
    // fix bug 31324,判断当前操作是否是清空回收站，是就在结束时改变清空回收站状态
    bool bdoingcleartrash = DFileService::instance()->getDoClearTrashState();
    if (action == DFMGlobal::DeleteAction && bdoingcleartrash && list.count() == 1 &&
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
    if (action == DFMGlobal::DeleteAction) {
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
            } else {
                moveToThread(qApp->thread());
            }
        }

        ~ErrorHandle() override
        {
            if (timer_id > 0) {
                killTimer(timer_id);
            }
            dialogManager->taskDialog()->removeTaskJob(fileJob.data());
            fileJob->disconnect();
            fileJob.reset(nullptr);
            qDebug() << "file copy error handle release!";
        }

        // 处理任务对话框显示之前的错误, 无法处理的错误将立即弹出对话框处理
        DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                             const DAbstractFileInfoPointer sourceInfo,
                                             const DAbstractFileInfoPointer targetInfo) override
        {
            if (slient) {
                return DFileCopyMoveJob::SkipAction;
            }

            if (error == DFileCopyMoveJob::DirectoryExistsError || error == DFileCopyMoveJob::FileExistsError) {
                if (sourceInfo->fileUrl() == targetInfo->fileUrl() ||
                        DStorageInfo::isSameFile(sourceInfo->fileUrl().path(), targetInfo->fileUrl().path())) {
                    return DFileCopyMoveJob::CoexistAction;
                }
            }

            if (timer_id > 0) {
                killTimer(timer_id);
                timer_id = 0;
            }

            DFileCopyMoveJob::Handle *handle = dialogManager->taskDialog()->addTaskJob(job, true);
            // fix bug 62822 设置当前进度条已显示，来显示进度到100%
            fileJob->setProgressShow(true);
            DUrl fromUrl = sourceInfo ? sourceInfo->fileUrl() : DUrl();
            DUrl toUrl = targetInfo ? targetInfo->fileUrl() : DUrl();
            emit job->currentJobChanged(fromUrl, toUrl, true);

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
            if (!fileJob->isFinished() && fileJob->isCanShowProgress()) {
                dialogManager->taskDialog()->addTaskJob(fileJob.data(), true);
                // fix bug 62822 设置当前进度条已显示，来显示进度到100%
                fileJob->setProgressShow(true);
                emit fileJob->currentJobChanged(currentJob.first, currentJob.second, false);
            }
            //在移动到同一个目录时，先不现实进度条，当有其他目录到同一个目录时，才会去显示
            if (!fileJob->isFinished() && !fileJob->isCanShowProgress()) {
                timer_id = startTimer(1000);
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

    DFileCopyMoveJob::Mode mode = DFileCopyMoveJob::CopyMode;
    switch (action) {
    case DFMGlobal::CopyAction:
        mode = DFileCopyMoveJob::CopyMode;
        break;
    case DFMGlobal::CutAction:
        mode = DFileCopyMoveJob::CutMode;
        break;
    case DFMGlobal::DeleteAction:
        mode = DFileCopyMoveJob::MoveMode;
        break;
    case DFMGlobal::RemoteAction:
        mode = DFileCopyMoveJob::RemoteMode;
        break;
    default:
        mode = DFileCopyMoveJob::UnknowMode;
    }
    job->setMode(mode);

    job->start(list, target);
    //走以前的老流程，阻塞主线去拷贝或者删除
    if (bold) {
        job->wait();

        QTimer::singleShot(200, dialogManager->taskDialog(), [job] {
            dialogManager->taskDialog()->removeTaskJob(job.data());
        });
        //fix bug 31324,判断当前操作是否是清空回收站，是就在结束时改变清空回收站状态
        if (action == DFMGlobal::CutAction && bdoingcleartrash && list.count() == 1 &&
                list.first().toString().endsWith(".local/share/Trash/files")) {
            DFileService::instance()->setDoClearTrashState(false);
        }

        if (slient) {
            error_handle->deleteLater();
        } else {
            QMetaObject::invokeMethod(error_handle, "deleteLater");
        }

        return job->targetUrlList();
    }
    //fix bug 35855走新流程不去阻塞主线程，拷贝线程自己去运行，主线程返回，当拷贝线程结束了再去处理以前的相应处理
    connect(job.data(), &QThread::finished, dialogManager->taskDialog(), [this, thisJob, error_handle, slient, event] {
        dialogManager->taskDialog()->removeTaskJob(thisJob);
        DUrlList targetUrlList = thisJob->targetUrlList();
        if (slient)
        {
            error_handle->deleteLater();
        } else
        {
            QMetaObject::invokeMethod(error_handle, "deleteLater");
        }
        //处理复制、粘贴和剪切(拷贝)结束后操作 fix bug 35855
        this->dealpasteEnd(targetUrlList, event);
        this->dealpasteEndV2(thisJob->sourceUrlList(), targetUrlList, event);
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

    if (event->action() == DFMGlobal::CopyAction || event->action() == DFMGlobal::RemoteAction) {
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, valid_files, true), true);
    } else {
        const QString targetDir(QFileInfo(event->urlList().first().toLocalFile()).absolutePath());

        if (targetDir.isEmpty()) {
            //到dfileservice里面作处理
            DFileService::instance()->dealPasteEnd(event, list);
            return;
        }
        //! 新增剪切回收站路径event->urlList()
        if (targetDir.startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, valid_files, false, event->urlList()), true);
        } else {
            DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromLocalFile(targetDir), valid_files), true);
        }
    }

    //到dfileservice里面作处理
    DFileService::instance()->dealPasteEnd(event, list);
}

void FileController::dealpasteEndV2(const DUrlList &srcUrlList, const DUrlList &targetUrlList, const QSharedPointer<DFMPasteEvent> &event) const
{
    Q_UNUSED(event)

    // 判断目标目录是否是保险箱目录如果是，发送信号激活计算保险大小的线程
    if(!targetUrlList.isEmpty() && targetUrlList.at(0).toLocalFile().contains(VaultController::makeVaultLocalPath()))
        emit VaultController::ins()->sigFinishedCopyFile();

    // 光盘缓存文件映射
    if (!targetUrlList.isEmpty() && targetUrlList.at(0).burnIsOnLocalStaging())
        MasteredMediaController::mapStagingFilesPath(srcUrlList, targetUrlList);

    // audit log
    if (!targetUrlList.isEmpty() && srcUrlList.size() >= targetUrlList.size())
        DFileService::instance()->dealPastedAudit(srcUrlList, targetUrlList);
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


    bool ok = !pasteFilesV2(nullptr, DFMGlobal::DeleteAction, event->fileUrlList(), DUrl(), event->silent(), event->force(), true).isEmpty();
    for (const auto &url : event->fileUrlList()) {
        if (url.toLocalFile().contains("/mtp:")) {
            DUrl mtpUrl(url);
            mtpUrl.setScheme(MTP_SCHEME);
            DAbstractFileWatcher::ghostSignal(mtpUrl.parentUrl(), &DAbstractFileWatcher::fileDeleted, mtpUrl);
        }
    }
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
    QSharedPointer<FileJob> job(new FileJob(FileJob::Trash));
    job->setWindowId(static_cast<int>(event->windowId()));
    dialogManager->addJob(job);
    DUrlList list = job->doMoveToTrash(event->urlList());
    dialogManager->removeJob(job->getJobId());

    // save event
    const QVariant &result = DFMEventDispatcher::instance()->processEvent<DFMGetChildrensEvent>(event->sender(), DUrl::fromTrashFile("/"),
                                                                                                QStringList(), QDir::AllEntries | QDir::Hidden | QDir::System);
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
            QSharedPointer<FileJob> job(new FileJob(FileJob::Move));
            job->setWindowId(static_cast<int>(event->windowId()));
            dialogManager->addJob(job);

            list = job->doMove(urlList, event->targetUrl());
            dialogManager->removeJob(job->getJobId());
        }

        DFMGlobal::clearClipboard();
    } else {
        QSharedPointer<FileJob> job(new FileJob(FileJob::Copy));
        job->setWindowId(static_cast<int>(event->windowId()));
        dialogManager->addJob(job);

        list = job->doCopy(urlList, event->targetUrl());
        dialogManager->removeJob(job->getJobId());
    }

    return list;
}

DUrlList FileController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    //新代码,修改复制拷贝流程，拷贝线程不去阻塞主线程，拷贝线程自己去处理，主线程直接返回，拷贝线程结束了在去处理以前的后续操作

    bool use_old_filejob = false;

#ifdef SW_LABEL
    /*fix bug 38276 【服务器UOS】【sw64】【SP1 update1(B11)】【DDE-UOS】【文件管理器】 复制文件到有相同的文件名存在的目录时，弹窗提示异常
         *针对申威平台做针对性处理，如果io文件存在即内核做了优化则走pasteFilesV2函数，否则走pasteFilesV1
        */
    use_old_filejob = !QFile("/proc/thread-self/io").exists();
#endif
    // 将最近使用文件url转换为本地url
    DUrlList urlList = event->urlList();
    if (!urlList.isEmpty() && urlList.first().isRecentFile()) {
        for (auto &url : urlList) {
            const auto &local = url.path();
            url = DUrl::fromLocalFile(local);
        }
        event->setData(urlList);
    }

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

    bool ok = QDir::current().mkpath(event->url().toLocalFile());

    if (ok) {
        fileAdded(event->url());
        DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));
    } else {
        // 创建文件夹失败，提示错误信息
        QString strErr = tr("Unable to create files here: %1").arg(strerror(errno));
        DThreadUtil::runInMainThread(dialogManager, &DialogManager::showMessageDialog,
                                     DialogManager::msgWarn, strErr, "", tr("Confirm","button"));
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
        fileAdded(event->url());
    } else {
        // 创建文件失败，提示错误信息
        QString strErr = tr("Unable to create files here: %1").arg(strerror(errno));
        dialogManager->showMessageDialog(DialogManager::msgWarn, strErr);

        return false;
    }

    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>(event, dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << event->url(), true));

    return true;
}

bool FileController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    QFile file(event->url().toLocalFile());

    event->accept();

    if (file.setPermissions(event->permissions())) {
        // 如果是快捷方式，发送文件属性变化信号，重绘item
        QFileInfo fileInfo(file);
        if (fileInfo.isSymLink())
            DAbstractFileWatcher::ghostSignal(event->url().parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, event->url());
        return true;
    }
    return false;
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
//        query.addQueryItem("locate_url", locateUrl);
        //为防止locateUrl传入QUrl被转码，locateUrl统一保存为base64
        QByteArray ba = locateUrl.toLocal8Bit().toBase64();
        query.addQueryItem("locate_url", ba);

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
    const DUrl url =  event->fileUrl();
    if (FileUtils::isGvfsMountFile(url.toLocalFile())) {
        return new DGIOFileDevice(url);
    }

    DLocalFileDevice *device = new DLocalFileDevice();

    device->setFileUrl(url);

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
    explicit Match(const QString &group)
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
        // 这里可能会析构 先复制一份再循环
        const QList<QPair<QString, QString>> patternListNew = patternList;
        for (auto pattern : patternListNew) {
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
    DUrl url = handleTagFileUrl(event->url());
    if (event->tags().isEmpty()) {
        const QStringList &tags = TagManager::instance()->getTagsThroughFiles({url});

        return tags.isEmpty() || TagManager::instance()->removeTagsOfFiles(tags, {url});
    }

    // 修复bug-59180
    // 保险箱文件不要创建标签
    if (VaultController::isVaultFile(event->url().toLocalFile()))
        return true;

    return TagManager::instance()->makeFilesTags(event->tags(), {url});
}

bool FileController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    DUrl url = handleTagFileUrl(event->url());
    return TagManager::instance()->removeTagsOfFiles(event->tags(), {url});
}

QList<QString> FileController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList urlList;
    for (auto url : event->urlList())
        urlList << handleTagFileUrl(url);

    return TagManager::instance()->getTagsThroughFiles(urlList);
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

bool FileController::fileAdded(const DUrl &url) const
{
    // 华为平台特有的问题，inotify无法向mtp发送信号，因此采用以下模式在文件创建完成后来刷新模型
    if (url.toLocalFile().contains("/mtp:")) {
        return DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::subfileCreated, url);
    }
    return true;
}

DUrl FileController::handleTagFileUrl(const DUrl &url) const
{
    DUrl newUrl(url);
    if (newUrl.path().startsWith("/data/home/"))
        newUrl.setPath(url.path().remove(0, sizeof("/data") - 1));

    return newUrl;
}

bool FileController::doHiddenFileRemind(const QString &name, bool *checkRule /*= nullptr*/)
{
    if(!name.startsWith("."))
        return true;
    if(DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool())
        return true;

    if(checkRule)
        *checkRule = true;
    const QVariant &oper = DFMApplication::instance()->genericAttribute(DFMApplication::GA_RenameHideFileOperate);
    if(oper.isValid()) {
        const int &value = oper.toInt();
        if(1 == value)
            return true;
        else if (2 == value)
            return false;
    }

    // show dialog
    auto result = dialogManager->showRenameNameDotBeginDialog();
    const bool noAsk = result.first;
    const bool hide = result.second;
    if(noAsk)
        DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_RenameHideFileOperate, hide ? 1 : 2);
    return hide;
}

FileDirIterator::FileDirIterator(const QString &path, const QStringList &nameFilters,
                                 QDir::Filters filter, QDirIterator::IteratorFlags flags, const bool gvfs)
    : DDirIterator()
    , filters(filter)
{
    bool sort_inode = flags.testFlag(static_cast<QDirIterator::IteratorFlag>(DDirIterator::SortINode));

    if (sort_inode) {
        iterator = new DFMSortInodeDirIterator(path);
    } else {
        iterator = new DFMQDirIterator(path, nameFilters, filter, flags, gvfs);
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
        const_cast<FileDirIterator *>(this)->nextInofCached.insert(info->fileUrl(), info);
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
    DAbstractFileInfoPointer newinfo = const_cast<FileDirIterator *>\
                                       (this)->nextInofCached.value(iterator->fileUrl());
    if (newinfo) {
        const_cast<FileDirIterator *>\
        (this)->nextInofCached.remove(iterator->fileUrl());
        return newinfo;
    }
    return iterator->fileInfo();
}

DUrl FileDirIterator::url() const
{
    return iterator->url();
}

void FileDirIterator::close()
{
    if (iterator)
        iterator->close();
}

bool FileDirIterator::enableIteratorByKeyword(const QString &keyword)
{
    Q_UNUSED(keyword)

    return false;
}
