/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "vaultcontroller.h"
#include "models/vaultfileinfo.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfileproxywatcher.h"
#include "vaulthandle.h"
#include "vaulterrorcode.h"
#include "dfmeventdispatcher.h"
#include "dfilestatisticsjob.h"
#include "log/dfmLogManager.h"
#include "log/filterAppender.h"
#include "vaultconfig.h"
#include "../vault/operatorcenter.h"
#include "../vault/vaulthelper.h"
#include "dfmapplication.h"

#include "appcontroller.h"
#include "singleton.h"
#include "dstorageinfo.h"
#include "models/desktopfileinfo.h"

#include "tag/tagmanager.h"
#include "shutil/fileutils.h"
#include "shutil/dfmfilelistfile.h"
#include "usershare/shareinfo.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"

#include "dfmevent.h"
#include "../vault/vaultglobaldefine.h"
#include "dbusinterface/vaultbruteforceprevention_interface.h"

#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <qplatformdefs.h>

#include <unistd.h>

VaultController *VaultController::cryfs = nullptr;
bool VaultController::m_isBigFileDeleting = false;

class VaultControllerPrivate
{
public:
    explicit VaultControllerPrivate(VaultController *cryFs);
    ~VaultControllerPrivate();

    CryFsHandle *m_cryFsHandle = nullptr;

private:
    VaultController *q_ptr;
    Q_DECLARE_PUBLIC(VaultController)
};

VaultControllerPrivate::VaultControllerPrivate(VaultController *cryFs)
    : q_ptr(cryFs)
{
}

VaultControllerPrivate::~VaultControllerPrivate()
{
    if (m_cryFsHandle) {
        delete m_cryFsHandle;
        m_cryFsHandle = nullptr;
    }
}

class VaultDirIterator : public DDirIterator
{
public:
    VaultDirIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    ~VaultDirIterator() override;

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    QDir::Filters filters;
    DFMFileListFile *hiddenFiles = nullptr;
    QDirIterator *iterator;
    bool nextIsCached = false;
};

VaultDirIterator::VaultDirIterator(const DUrl &url, const QStringList &nameFilters,
                                   QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator(), filters(filter)
{
    QString path = VaultController::vaultToLocal(url);
    iterator = new QDirIterator(path, nameFilters, filter, flags);

    hiddenFiles = new DFMFileListFile(path);
}

VaultDirIterator::~VaultDirIterator()
{
    if (iterator) {
        delete iterator;
    }

    if (hiddenFiles) {
        delete hiddenFiles;
    }
}

DUrl VaultDirIterator::next()
{
    if (nextIsCached) {
        nextIsCached = false;

        QString path = iterator->filePath();
        DUrl url = VaultController::localToVault(path);
        VaultController::ins()->updateFileInfo(DUrlList() << url);
        return url;
    }

    DUrl url = VaultController::localToVault(iterator->next());
    VaultController::ins()->updateFileInfo(DUrlList() << url);
    return url;
}

// 添加过滤，将保险箱中.hidden中记录的文件隐藏
bool VaultDirIterator::hasNext() const
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
        const_cast<VaultDirIterator *>(this)->iterator->next();
        QString absoluteFilePath = iterator->fileInfo().absoluteFilePath();
        info = DAbstractFileInfoPointer(new VaultFileInfo(DUrl::fromLocalFile(absoluteFilePath)));

        if (!info->isPrivate() && (showHidden || (!info->isHidden() && !hiddenFiles->contains(info->fileName())))) {
            break;
        }

        info.reset();
    } while (iterator->hasNext());

    // file is exists
    if (info) {
        const_cast<VaultDirIterator *>(this)->nextIsCached = true;

        return true;
    }

    return false;
}

QString VaultDirIterator::fileName() const
{
    return iterator->fileName();
}

DUrl VaultDirIterator::fileUrl() const
{
    return VaultController::localToVault(iterator->filePath());
}

const DAbstractFileInfoPointer VaultDirIterator::fileInfo() const
{
    return DFileService::instance()->createFileInfo(nullptr, fileUrl());
}

DUrl VaultDirIterator::url() const
{
    return fileUrl();
}

VaultController::VaultController(QObject *parent)
    : DAbstractFileController(parent),
      d_ptr(new VaultControllerPrivate(this)),
      m_recordVaultPageMark(VaultPageMark::UNKNOWN),
      m_vaultVisiable(true)
{
    Q_D(VaultController);

    //! 屏蔽保险箱内的文件信息写入到日志文件
    DFMLogManager::getFilterAppender()->addFilter(VAULT_DECRYPT_DIR_NAME);

    d->m_cryFsHandle = new CryFsHandle();

    connect(this, &VaultController::sigCreateVault, d->m_cryFsHandle, &CryFsHandle::createVault);
    connect(this, &VaultController::sigUnlockVault, d->m_cryFsHandle, &CryFsHandle::unlockVault);
    connect(this, &VaultController::sigLockVault, d->m_cryFsHandle, &CryFsHandle::lockVault);

    // 创建保险箱，关联信号
    connect(d->m_cryFsHandle, &CryFsHandle::signalCreateVault, this, &VaultController::slotCreateVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalUnlockVault, this, &VaultController::slotUnlockVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalLockVault, this, &VaultController::slotLockVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalReadError, this, &VaultController::signalReadError);
    connect(d->m_cryFsHandle, &CryFsHandle::signalReadOutput, this, &VaultController::signalReadOutput);
    connect(this, &VaultController::sigFinishedCopyFile, this, &VaultController::slotFinishedCopyFileTotalSize);

    // Get root dir size.
    m_sizeWorker = new DFileStatisticsJob(this);

    slotFinishedCopyFileTotalSize();

    connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, this, &VaultController::updateFolderSizeLabel);

    // Refresh size when lock state changed.
    connect(this, &VaultController::signalUnlockVault, this, &VaultController::refreshTotalSize);
    connect(this, &VaultController::signalLockVault, this, &VaultController::refreshTotalSize);

    // 保险箱大小计算线程结束后，再次计算一次大小
    connect(m_sizeWorker, &QThread::finished, this, &VaultController::onFinishCalcSize);

    // 初始化时，记录保险箱状态
    m_enVaultState = state();

    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if (pTaskDlg) {
        connect(pTaskDlg, &DTaskDialog::paused, this, &VaultController::taskPaused);
    }

    m_vaultInterface = new VaultBruteForcePreventionInterface("com.deepin.filemanager.daemon",
                                                              "/com/deepin/filemanager/daemon/VaultManager2",
                                                              QDBusConnection::systemBus(),
                                                              this);

    slotVaultPolicy();

    QDBusConnection::systemBus().connect("com.deepin.filemanager.daemon",
                                         "/com/deepin/filemanager/daemon/AccessControlManager",
                                         "com.deepin.filemanager.daemon.AccessControlManager",
                                         "AccessVaultPolicyNotify",
                                         this,
                                         SLOT(slotVaultPolicy()));
}

VaultController *VaultController::ins()
{
    if (!cryfs) {
        DUrl url(DFMVAULT_ROOT);
        url.setScheme(DFMVAULT_SCHEME);
        QList<DAbstractFileController *> vaultObjlist = DFileService::getHandlerTypeByUrl(url);
        if (vaultObjlist.size() > 0)
            cryfs = static_cast<VaultController *>(vaultObjlist.first());
    }
    return cryfs;
}

const DAbstractFileInfoPointer VaultController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    DUrl url(makeVaultUrl());

    if (url == event->url()) {
        return DAbstractFileInfoPointer(new VaultFileInfo(makeVaultUrl(makeVaultLocalPath())));
    }

    //! 在保险箱中判断是否有桌面专属文件
    url = event->url();
    QString localFile = url.toLocalFile();
    QFileInfo info(localFile);

    if (!info.isSymLink() && FileUtils::isDesktopFile(localFile)) {
        //! 创建桌面文件信息
        return DAbstractFileInfoPointer(new DesktopFileInfo(event->url()));
    }

    const_cast<VaultController *>(this)->updateFileInfo(DUrlList() << event->url());
    return DAbstractFileInfoPointer(new VaultFileInfo(event->url()));
}

const DDirIteratorPointer VaultController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    DUrl url = event->url();
    if (event->url() == makeVaultUrl()) {
        url = makeVaultUrl(makeVaultLocalPath());
    }
    return DDirIteratorPointer(new VaultDirIterator(url, event->nameFilters(), event->filters(), event->flags()));
}

DAbstractFileWatcher *VaultController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    QString urlPath = event->url().toLocalFile();
    if (urlPath.isEmpty()) return nullptr;

    DUrl url = makeVaultUrl(urlPath);
    auto watcher = new DFileProxyWatcher(url,
                                         new DFileWatcher(urlPath),
                                         VaultController::localUrlToVault);

    connect(watcher, &DFileProxyWatcher::fileDeleted, this, &VaultController::refreshTotalSize);
    connect(watcher, &DFileProxyWatcher::subfileCreated, this, &VaultController::refreshTotalSize);
    connect(watcher, &DFileProxyWatcher::fileMoved, this, &VaultController::refreshTotalSize);
    connect(watcher, &DFileProxyWatcher::fileAttributeChanged, this, &VaultController::refreshTotalSize);

    return watcher;
}

bool VaultController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), vaultToLocalUrl(event->url()));
}

bool VaultController::openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const
{
    //    return DFileService::instance()->openFiles(event->sender(), vaultToLocalUrls(event->urlList()));
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
        QString url = vaultToLocal(fileUrl);
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
                AppController::instance()->actionOpenWithCustom(dMakeEventPointer<DFMOpenFileEvent>(event->sender(), fileUrl));   // requestShowOpenWithDialog
            }
        }
    }

    return result;
}

bool VaultController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    //处理快捷方式，还原成原路径
    DUrl fileUrl = event->url();

    const DAbstractFileInfoPointer pfile = createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(this, fileUrl));

    if (pfile->isSymLink()) {
        const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, pfile->symLinkTarget());
        if (!linkInfo.data())
            return false;

        if (!linkInfo->exists()) {
            dialogManager->showBreakSymlinkDialog(linkInfo->fileName(), fileUrl);
            return false;
        }
        const_cast<DUrl &>(fileUrl) = linkInfo->redirectedFileUrl();
    }
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), vaultToLocalUrl(fileUrl));
}

bool VaultController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
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

bool VaultController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    const_cast<VaultController *>(this)->slotFinishedCopyFileTotalSize();

    const_cast<VaultController *>(this)->setVauleCurrentPageMark(VaultPageMark::DELETEFILEPAGE);
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    bool bDeletedSuccess = DFileService::instance()->deleteFiles(event->sender(), urlList, true, event->silent());
    if (bDeletedSuccess) {
        const_cast<VaultController *>(this)->updateFileInfo(urlList);
    }
    // 修复bug-89733 保险箱删除2个及以上的大文件时，删除完成后，界面不刷新
    // 删除两个及以上的大文件时，刷新操作非常卡顿，所以不在刷新，并且模态弹框（与产品沟通后的做法）
    // 这就导致了上述bug的产生，此处修改，增加判断，如果是此种情况，删除完成后，刷新一次界面
    if (m_isBigFileDeleting)
        emit fileSignalManager->requestFreshAllFileView();
    m_isBigFileDeleting = false;
    return true;
}

DUrlList VaultController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    const_cast<VaultController *>(this)->slotFinishedCopyFileTotalSize();
    const_cast<VaultController *>(this)->setVauleCurrentPageMark(VaultPageMark::DELETEFILEPAGE);
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    bool bDeletedSuccess = DFileService::instance()->deleteFiles(event->sender(), urlList);
    if (bDeletedSuccess) {
        const_cast<VaultController *>(this)->updateFileInfo(urlList);
    }
    // 修复bug-89733 保险箱删除2个及以上的大文件时，删除完成后，界面不刷新
    // 删除两个及以上的大文件时，刷新操作非常卡顿，所以不在刷新，并且模态弹框（与产品沟通后的做法）
    // 这就导致了上述bug的产生，此处修改，增加判断，如果是此种情况，删除完成后，刷新一次界面
    if (m_isBigFileDeleting)
        emit fileSignalManager->requestFreshAllFileView();
    m_isBigFileDeleting = false;
    return urlList;
}

DUrlList VaultController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    const_cast<VaultController *>(this)->slotFinishedCopyFileTotalSize();
    const_cast<VaultController *>(this)->setVauleCurrentPageMark(VaultPageMark::COPYFILEPAGE);
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    DUrl url = vaultToLocalUrl(event->targetUrl());
    DUrlList ulist = DFileService::instance()->pasteFile(event->sender(), event->action(), url, urlList);
    return ulist;
}

bool VaultController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    const_cast<VaultController *>(this)->slotFinishedCopyFileTotalSize();
    const_cast<VaultController *>(this)->setVauleCurrentPageMark(VaultPageMark::CLIPBOARDPAGE);
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), urlList);
}

bool VaultController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    bool flg = DFileService::instance()->renameFile(event->sender(),
                                                    vaultToLocalUrl(event->fromUrl()),
                                                    vaultToLocalUrl(event->toUrl()));
    if (flg) {
        const_cast<VaultController *>(this)->updateFileInfo(DUrlList() << event->fromUrl());
    }
    return flg;
}

bool VaultController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    const QString &current_dir = QDir::currentPath();

    QDir::setCurrent(vaultToLocalUrl(event->url()).toLocalFile());

    bool ok = QProcess::startDetached(FileUtils::defaultTerminalPath());

    QDir::setCurrent(current_dir);

    return ok;
}

bool VaultController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
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

bool VaultController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->deleteFiles(nullptr, { DUrl::fromBookMarkFile(event->url(), QString()) }, false);
}

bool VaultController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    QString path = vaultToLocalUrl(event->fileUrl()).path();
    QFile file(path);

    QUrl url = event->toUrl().toLocalFile();

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

bool VaultController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    DUrl url = event->url();
    DUrl durl = vaultToLocalUrl(url);
    QList<QString> taglist = event->tags();
    if (taglist.isEmpty()) {
        const QStringList &tags = TagManager::instance()->getTagsThroughFiles({ durl });

        return tags.isEmpty() || TagManager::instance()->removeTagsOfFiles(tags, { durl });
    }

    return TagManager::instance()->makeFilesTags(taglist, { durl });
}

bool VaultController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    DUrl url = event->url();
    DUrl durl = vaultToLocalUrl(url);
    QList<QString> taglist = event->tags();
    return TagManager::instance()->removeTagsOfFiles(taglist, { durl });
}

QList<QString> VaultController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList urllist = event->urlList();
    DUrlList tempList = vaultToLocalUrls(urllist);
    return TagManager::instance()->getTagsThroughFiles(tempList);
}

bool VaultController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    DUrl url = event->url();
    DUrl durl = vaultToLocalUrl(url);
    bool flg = DFileService::instance()->setPermissions(event->sender(), durl, event->permissions());
    if (flg) {
        const_cast<VaultController *>(this)->updateFileInfo(DUrlList() << url);
    }

    return flg;
}

void VaultController::updateFileInfo(const DUrlList &fileUrls)
{
    static QMutex mutex;
    mutex.lock();
    for (const auto &url : fileUrls) {
        QFileInfo fileInfo(url.path());
        if (!fileInfo.exists()) {
            m_mapVaultFileInfo.remove(url);

            //当文件删除时，删除隐藏文件集中的隐藏
            QString absort = url.path().left(url.path().length() - url.fileName().length());
            DFMFileListFile flf(absort);
            if (flf.contains(url.fileName())) {
                flf.remove(url.fileName());
                flf.save();
            }
        } else {
            if (!m_mapVaultFileInfo.contains(url)) {
                FileBaseInfo fbi;
                fbi.isExist = true;
                fbi.isDir = fileInfo.isDir();
                fbi.isFile = fileInfo.isFile();
                fbi.isSymLink = fileInfo.isSymLink();
                m_mapVaultFileInfo.insert(url, fbi);
            } else {
                m_mapVaultFileInfo[url].isDir = fileInfo.isDir();
                m_mapVaultFileInfo[url].isFile = fileInfo.isFile();
                m_mapVaultFileInfo[url].isSymLink = fileInfo.isSymLink();
            }
        }
    }
    mutex.unlock();
}

VaultController::FileBaseInfo VaultController::getFileInfo(const DUrl &fileUrl)
{
    if (m_mapVaultFileInfo.contains(fileUrl)) {
        return m_mapVaultFileInfo[fileUrl];
    }

    return FileBaseInfo();
}

DUrl VaultController::makeVaultUrl(QString path, QString host)
{
    Q_UNUSED(host)
    // blumia: if path is not start with a `/`, QUrl::setPath will destory the whole QUrl
    //         and only leave the path to the QUrl.
    if (!path.startsWith('/')) {
        path = '/' + path;
    }

    DUrl newUrl;
    newUrl.setScheme(DFMVAULT_SCHEME);
    newUrl.setHost(host);
    newUrl.setPath(path);
    return newUrl;
}

DUrl VaultController::localUrlToVault(const DUrl &vaultUrl)
{
    return VaultController::localToVault(vaultUrl.path());
}

DUrl VaultController::localToVault(QString localPath)
{
    if (isVaultFile(localPath)) {
        return VaultController::makeVaultUrl(localPath);
    } else {
        return DUrl();
    }
}

QString VaultController::vaultToLocal(const DUrl &vaultUrl)
{
    if (vaultUrl.scheme() == DFMVAULT_SCHEME) {
        if (vaultUrl == makeVaultUrl("/"))
            return makeVaultLocalPath(vaultUrl.path());
        else {
            return vaultUrl.toLocalFile();
        }
    }

    return vaultUrl.toLocalFile();
}

DUrl VaultController::vaultToLocalUrl(const DUrl &vaultUrl)
{
    if (vaultUrl.scheme() != DFMVAULT_SCHEME) return vaultUrl;
    return DUrl::fromLocalFile(vaultToLocal(vaultUrl));
}

DUrlList VaultController::vaultToLocalUrls(DUrlList vaultUrls)
{
    std::transform(vaultUrls.begin(), vaultUrls.end(), vaultUrls.begin(), [](const DUrl &url) {
        return vaultToLocalUrl(url);
    });

    return vaultUrls;
}

VaultController::VaultState VaultController::state(QString lockBaseDir)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        // 记录保险箱状态
        m_enVaultState = NotAvailable;
        return NotAvailable;
    }

    if (lockBaseDir.isEmpty()) {
        lockBaseDir = makeVaultLocalPath("cryfs.config", VAULT_ENCRYPY_DIR_NAME);
    } else {
        if (lockBaseDir.endsWith("/"))
            lockBaseDir += "cryfs.config";
        else
            lockBaseDir += "/cryfs.config";
    }
    if (QFile::exists(lockBaseDir)) {
        QStorageInfo info(makeVaultLocalPath(""));
        QString temp = info.fileSystemType();
        if (info.isValid() && temp == "fuse.cryfs") {
            m_enVaultState = Unlocked;
            return Unlocked;
        }
        m_enVaultState = Encrypted;
        return Encrypted;
    } else {
        m_enVaultState = NotExisted;
        return NotExisted;
    }
}

bool VaultController::isRootDirectory(QString path)
{
    bool bRootDir = false;
    QString localFilePath = makeVaultLocalPath();
    QString pathNoSplash = localFilePath;
    pathNoSplash.chop(1);
    if (localFilePath == path || makeVaultUrl().toString() == path
        || pathNoSplash == path) {
        bRootDir = true;
    }
    return bRootDir;
}

QString VaultController::getErrorInfo(int state)
{
    QString strErr("");
    switch (state) {
    case 10:
        strErr = "The command line arguments are invalid.";
        break;
    case 11:
        strErr = "Couldn't load config file. Probably the password is wrong";
        break;
    case 12:
        strErr = "Password cannot be empty";
        break;
    case 13:
        strErr = "The file system format is too new for this CryFS version. Please update your CryFS version.";
        break;
    case 14:
        strErr = "The file system format is too old for this CryFS version. Run with --allow-filesystem-upgrade to upgrade it.";
        break;
    case 15:
        strErr = "The file system uses a different cipher than the one specified on the command line using the --cipher argument.";
        break;
    case 16:
        strErr = "Base directory doesn't exist or is inaccessible (i.e. not read or writable or not a directory)";
        break;
    case 17:
        strErr = "Mount directory doesn't exist or is inaccessible (i.e. not read or writable or not a directory)";
        break;
    case 18:
        strErr = "Base directory can't be a subdirectory of the mount directory";
        break;
    case 19:
        strErr = "Something's wrong with the file system.";
        break;
    case 20:
        strErr = "The filesystem id in the config file is different to the last time we loaded a filesystem from this basedir. This could mean an attacker replaced the file system with a different one. You can pass the --allow-replaced-filesystem option to allow this.";
        break;
    case 21:
        strErr = "The filesystem encryption key differs from the last time we loaded this filesystem. This could mean an attacker replaced the file system with a different one. You can pass the --allow-replaced-filesystem option to allow this.";
        break;
    case 22:
        strErr = "The command line options and the file system disagree on whether missing blocks should be treated as integrity violations.";
        break;
    case 23:
        strErr = "File system is in single-client mode and can only be used from the client that created it.";
        break;
    case 24:
        strErr = "A previous run of the file system detected an integrity violation. Preventing access to make sure the user notices. The file system will be accessible again after the user deletes the integrity state file.";
        break;
    case 25:
        strErr = "An integrity violation was detected and the file system unmounted to make sure the user notices.";
        break;
    case 26:
        strErr = "Mount directory is not empty.";
        break;
    case 27:
        strErr = "Mount directory in use.";
        break;
    case 28:
        strErr = "Cryfs not installed.";
        break;
    case 29:
        strErr = "Mount directory doesn't exist.";
        break;
    case 30:
        strErr = "Mounted directory encrypted.";
        break;
    case 31:
        strErr = "No permissions.";
        break;
    case 32:
        strErr = "Fusermount does not exist";
        break;
    case 33:
        strErr = "An encrypted folder created by Cryfs already exists.";
        break;
    default:
        break;
    }

    return strErr;
}

QString VaultController::toInternalPath(const QString &externalPath)
{
    QString ret = externalPath;
    DUrl url(externalPath);
    if (url.isVaultFile()) {
        QString path = url.toString();
        ret = path.replace(DFMVAULT_ROOT, VaultController::makeVaultUrl(VaultController::makeVaultLocalPath()).toString());
    }
    return ret;
}

QString VaultController::toExternalPath(const QString &internalPath)
{
    QString retPath = internalPath;
    QString vaultRootPath = VaultController::makeVaultUrl(VaultController::makeVaultLocalPath()).toString();
    retPath = retPath.replace(vaultRootPath, DFMVAULT_ROOT);

    return retPath;
}

QString VaultController::localPathToVirtualPath(const QString &localPath)
{
    if (localPath.isEmpty())
        return localPath;
    QString result(localPath);
    // 如果是目录，并且最后没有"/"，加上"/"
    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, DUrl::fromLocalFile(result));
    if (info && info->isDir()) {
        result = result.endsWith(QDir::separator()) ? result : result.append(QDir::separator());
    }
    return result.replace(VaultController::makeVaultLocalPath(), DFMVAULT_ROOT);
}

QString VaultController::virtualPathToLocalPath(const QString &virtualPath)
{
    if (virtualPath.isEmpty())
        return virtualPath;
    QString result(virtualPath);
    return result.replace(DFMVAULT_ROOT, VaultController::makeVaultLocalPath());
}

qint64 VaultController::totalsize() const
{
    return m_totalSize;
}

void VaultController::setBigFileIsDeleting(const bool isDeleting)
{
    m_isBigFileDeleting = isDeleting;
}

int VaultController::getLeftoverErrorInputTimes()
{
    createVaultBruteForcePreventionInterface();
    int leftChance = -1;
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<int> reply = m_vaultInterface->getLeftoverErrorInputTimes(int(getuid()));
        reply.waitForFinished();
        if (reply.isError()) {
            qInfo() << "Warning: Obtaining the remaining number of password input errors!" << reply.error().message();
        } else {
            leftChance = reply.value();
        }
    }
    return leftChance;
}

void VaultController::leftoverErrorInputTimesMinusOne()
{
    createVaultBruteForcePreventionInterface();
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = m_vaultInterface->leftoverErrorInputTimesMinusOne(int(getuid()));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: The remaining password input times minus 1 is wrong!" << reply.error().message();
    }
}

void VaultController::restoreLeftoverErrorInputTimes()
{
    createVaultBruteForcePreventionInterface();
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = m_vaultInterface->restoreLeftoverErrorInputTimes(int(getuid()));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error in restoring the remaining number of incorrect entries!" << reply.error().message();
    }
}

void VaultController::startTimerOfRestorePasswordInput()
{
    createVaultBruteForcePreventionInterface();
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = m_vaultInterface->startTimerOfRestorePasswordInput(int(getuid()));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error when opening the password input timer!" << reply.error().message();
    }
}

int VaultController::getNeedWaitMinutes()
{
    createVaultBruteForcePreventionInterface();
    int result = 100;
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<int> reply = m_vaultInterface->getNeedWaitMinutes(int(getuid()));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Failed to get the number of minutes to wait!" << reply.error().message();
        else
            result = reply.value();
    }
    return result;
}

void VaultController::restoreNeedWaitMinutes()
{
    createVaultBruteForcePreventionInterface();
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = m_vaultInterface->restoreNeedWaitMinutes(int(getuid()));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warnning: The restore needs to wait for a few minutes to fail!" << reply.error().message();
    }
}

void VaultController::updateFolderSizeLabel(const qint64 size) noexcept
{
    m_totalSize = size;
}

bool VaultController::isVaultFile(QString path)
{
    bool bVaultFile = false;
    QString rootPath = makeVaultLocalPath();

    if (rootPath.isEmpty())
        return false;

    if (rootPath.back() == "/") {
        rootPath.chop(1);
    }

    if (path.contains(rootPath) && path.left(6) != "search") {
        bVaultFile = true;
    }

    return bVaultFile;
}

QFileDevice::Permissions VaultController::getPermissions(QString filePath)
{
    QFileDevice::Permissions permissions;
    QT_STATBUF statBuffer;
    if (QT_STAT(QFile::encodeName(filePath), &statBuffer) == 0) {
        auto st_mode = statBuffer.st_mode;

        auto setPermission = [&](bool isOwner, QFileDevice::Permissions permission) {
            if (isOwner) {
                permissions |= permission;
            } else {
                permissions &= ~permission;
            }
        };

        setPermission(st_mode & S_IRUSR, QFileDevice::ReadOwner);
        setPermission(st_mode & S_IWUSR, QFileDevice::WriteOwner);
        setPermission(st_mode & S_IXUSR, QFileDevice::ExeOwner);

        setPermission(st_mode & S_IRUSR, QFileDevice::ReadUser);
        setPermission(st_mode & S_IWUSR, QFileDevice::WriteUser);
        setPermission(st_mode & S_IXUSR, QFileDevice::ExeUser);

        setPermission(st_mode & S_IRGRP, QFileDevice::ReadGroup);
        setPermission(st_mode & S_IWGRP, QFileDevice::WriteGroup);
        setPermission(st_mode & S_IXGRP, QFileDevice::ExeGroup);

        setPermission(st_mode & S_IROTH, QFileDevice::ReadOther);
        setPermission(st_mode & S_IWOTH, QFileDevice::WriteOther);
        setPermission(st_mode & S_IXOTH, QFileDevice::ExeOther);
    }

    return permissions;
}

QString VaultController::pathToVirtualPath(QString path)
{
    QString nextPath = path;
    int index = nextPath.indexOf(VAULT_DECRYPT_DIR_NAME);
    if (index == -1) {
        // fallback to vault file root dir.
        return VaultController::makeVaultUrl("/").toString();
    }

    index += QString(VAULT_DECRYPT_DIR_NAME).length();

    return VaultController::makeVaultUrl(nextPath.mid(index)).toString();
}

DUrl VaultController::urlToVirtualUrl(QString path)
{
    QString nextPath = path;
    int index = nextPath.indexOf(VAULT_DECRYPT_DIR_NAME);
    if (index == -1) {
        // fallback to vault file root dir.
        return VaultController::makeVaultUrl("/");
    }

    index += QString(VAULT_DECRYPT_DIR_NAME).length();

    return VaultController::makeVaultUrl(nextPath.mid(index));
}

bool VaultController::isBigFileDeleting()
{
    return m_isBigFileDeleting;
}

void VaultController::createVault(const DSecureString &password, QString lockBaseDir, QString unlockFileDir)
{
    auto createIfNotExist = [](const QString &path) {
        if (!QFile::exists(path)) {
            QDir().mkpath(path);
        } else {   // 修复bug-52351 创建保险箱前，如果文件夹存在，则清空
            QDir dir(path);
            if (!dir.isEmpty()) {
                QDirIterator dirsIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot);
                while (dirsIterator.hasNext()) {
                    if (!dir.remove(dirsIterator.next())) {
                        QDir(dirsIterator.filePath()).removeRecursively();
                    }
                }
            }
        }
    };

    if (lockBaseDir.isEmpty() || unlockFileDir.isEmpty()) {
        if (state() != NotExisted) {
            emit signalCreateVault(static_cast<int>(ErrorCode::EncryptedExist));
            return;
        }

        createIfNotExist(makeVaultLocalPath("", VAULT_ENCRYPY_DIR_NAME));
        createIfNotExist(makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME));

        emit sigCreateVault(makeVaultLocalPath("", VAULT_ENCRYPY_DIR_NAME),
                            makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME),
                            password);
    } else {
        if (state(lockBaseDir) != NotExisted) {
            emit signalCreateVault(static_cast<int>(ErrorCode::EncryptedExist));
            return;
        }

        createIfNotExist(lockBaseDir);
        createIfNotExist(unlockFileDir);
        emit sigCreateVault(lockBaseDir, unlockFileDir, password);
    }
}

void VaultController::unlockVault(const DSecureString &password, QString lockBaseDir, QString unlockFileDir)
{
    // 修复bug-52351
    // 保险箱解锁前,创建挂载目录
    QString strPath;
    if (unlockFileDir.isEmpty()) {
        strPath = makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME);
    } else {
        strPath = unlockFileDir;
    }
    if (QFile::exists(strPath)) {   // 如果存在,则清空目录
        QDir dir(strPath);
        if (!dir.isEmpty()) {
            QDirIterator dirsIterator(strPath, QDir::AllEntries | QDir::NoDotAndDotDot);
            while (dirsIterator.hasNext()) {
                if (!dir.remove(dirsIterator.next())) {
                    QDir(dirsIterator.filePath()).removeRecursively();
                }
            }
        }
    } else {   // 如果不存在,则创建目录
        QDir().mkpath(strPath);
    }

    if (lockBaseDir.isEmpty() || unlockFileDir.isEmpty()) {
        if (state() != Encrypted) {
            emit signalUnlockVault(static_cast<int>(ErrorCode::MountpointNotEmpty));
            return;
        }

        emit sigUnlockVault(makeVaultLocalPath("", VAULT_ENCRYPY_DIR_NAME),
                            makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME),
                            password);
    } else {
        if (state(lockBaseDir) != Encrypted) {
            emit signalUnlockVault(static_cast<int>(ErrorCode::MountpointNotEmpty));
            return;
        }
        emit sigUnlockVault(lockBaseDir, unlockFileDir, password);
    }
}

void VaultController::lockVault(QString lockBaseDir, QString unlockFileDir)
{
    if (lockBaseDir.isEmpty() || unlockFileDir.isEmpty()) {
        if (state() != Unlocked) {
            emit signalLockVault(static_cast<int>(ErrorCode::MountdirEncrypted));
            return;
        }
        emit sigLockVault(makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME));
    } else {
        if (state(lockBaseDir) != Unlocked) {
            emit signalLockVault(static_cast<int>(ErrorCode::MountdirEncrypted));
            return;
        }
        emit sigLockVault(unlockFileDir);
    }
}

QString VaultController::makeVaultLocalPath(QString path, QString base)
{
    if (base.isEmpty()) {
        base = VAULT_DECRYPT_DIR_NAME;
    }
    return VAULT_BASE_PATH + QDir::separator() + base + (path.startsWith('/') ? "" : "/") + path;
}

QString VaultController::vaultLockPath()
{
    return makeVaultLocalPath("", VAULT_ENCRYPY_DIR_NAME);
}

QString VaultController::vaultUnlockPath()
{
    return makeVaultLocalPath("", VAULT_DECRYPT_DIR_NAME);
}

void VaultController::refreshTotalSize()
{
    // 修复BUG-42897 打开正在拷贝或剪贴的文件夹时，主界面卡死问题
    // 当保险箱大小计算线程没有结束时，直接返回
    if (m_sizeWorker->isRunning()) {
        m_bNeedRefreshSize = true;
        return;
    }

    DUrl url = vaultToLocalUrl(makeVaultUrl());
    m_sizeWorker->start({ url });
}

void VaultController::onFinishCalcSize()
{
    // 但保险箱大小计算完成后，再次计算一次保险箱的大小
    if (m_bNeedRefreshSize && !m_sizeWorker->isRunning()) {
        DUrl url = vaultToLocalUrl(makeVaultUrl());
        // 修复BUG-47507 增加判断，如果该线程正在启动，不要再次进入该线程
        m_sizeWorker->start({ url });
        m_bNeedRefreshSize = false;
    }
}

void VaultController::taskPaused(const DUrlList &src, const DUrl &dst)
{
    if (isVaultFile(dst.toLocalFile()) || (src.size() > 0 && isVaultFile(src.front().toLocalFile()))) {
        refreshTotalSize();
    }
}

// 创建保险箱，执行该槽函数,通知保险箱创建成功与否，并更新保险箱的状态
void VaultController::slotCreateVault(int state)
{
    if (state == static_cast<int>(ErrorCode::Success)) {
        m_enVaultState = Unlocked;
    }
    emit signalCreateVault(state);
}

void VaultController::slotUnlockVault(int state)
{
    if (state == static_cast<int>(ErrorCode::Success)) {
        m_enVaultState = Unlocked;
    }
    emit signalUnlockVault(state);
}

void VaultController::slotLockVault(int state)
{
    if (state == static_cast<int>(ErrorCode::Success)) {
        m_enVaultState = Encrypted;
        // 刷新下界面
        emit fileSignalManager->requestFreshAllFileView();
        VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
    }
    emit signalLockVault(state);
}

void VaultController::slotFinishedCopyFileTotalSize()
{
    if (!m_sizeWorker->isRunning()) {
        DUrl rootUrl = vaultToLocalUrl(makeVaultUrl());
        m_sizeWorker->start({ rootUrl });
    }
}

void VaultController::slotVaultPolicy()
{
    switch (getVaultPolicy()) {
    case INVISIBLE: {
        switch (getVaultCurrentPageMark()) {
        case VaultPageMark::UNKNOWN:
            break;
        case VaultPageMark::CREATEVAULTPAGE:
            emit sigCloseWindow();
            break;
        case VaultPageMark::RETRIEVEPASSWORDPAGE:
            emit sigCloseWindow();
            break;
        case VaultPageMark::VAULTPAGE:
            emit sigCloseWindow();
            break;
        case VaultPageMark::CLIPBOARDPAGE:
            if (m_vaultVisiable) {
                lockVault();
                m_vaultVisiable = false;
                emit DFMApplication::instance()->reloadComputerModel();
                VaultHelper::killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::COPYFILEPAGE:
            if (m_vaultVisiable) {
                lockVault();
                m_vaultVisiable = false;
                emit DFMApplication::instance()->reloadComputerModel();
                VaultHelper::killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::CREATEVAULTPAGE1:
        case VaultPageMark::UNLOCKVAULTPAGE:
        case VaultPageMark::DELETEFILEPAGE:
        case VaultPageMark::DELETEVAULTPAGE:
            setVaultPolicyState(2);
            return;
        }

        lockVault();
        m_vaultVisiable = false;
        emit DFMApplication::instance()->reloadComputerModel();
    } break;
    case VISIBLE:
        if (!m_vaultVisiable) {
            m_vaultVisiable = true;
            emit DFMApplication::instance()->reloadComputerModel();
        }
        break;
    }
}

bool VaultController::getVaultVersion()
{
    VaultConfig config;
    QString strVersion = config.get(CONFIG_NODE_NAME, CONFIG_KEY_VERSION).toString();
    if (!strVersion.isEmpty() && strVersion != CONFIG_VAULT_VERSION)
        return true;

    return false;
}

void VaultController::createVaultBruteForcePreventionInterface()
{
    // 防暴力破解功能的dbus对象
    if (!m_vaultInterface->isValid())
        m_vaultInterface = new VaultBruteForcePreventionInterface("com.deepin.filemanager.daemon",
                                                                  "/com/deepin/filemanager/daemon/VaultManager2",
                                                                  QDBusConnection::systemBus(),
                                                                  this);
}

int VaultController::getVaultPolicy()
{
    QDBusInterface deepin_systemInfo("com.deepin.filemanager.daemon",
                                     "/com/deepin/filemanager/daemon/AccessControlManager",
                                     "com.deepin.filemanager.daemon.AccessControlManager",
                                     QDBusConnection::systemBus(), this);

    int vaulthidestate = -1;

    //调用
    auto response = deepin_systemInfo.call("QueryVaultAccessPolicyVisible");
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        QVariantList value = response.arguments();
        if (!value.isEmpty()) {
            QVariant varVaule = value.first();
            vaulthidestate = varVaule.toInt();
        } else {
            vaulthidestate = -1;
        }

    } else {
        qDebug() << "value method called failed!";
        vaulthidestate = -1;
    }

    return vaulthidestate;
}

bool VaultController::setVaultPolicyState(int policyState)
{
    QDBusInterface deepin_systemInfo("com.deepin.filemanager.daemon",
                                     "/com/deepin/filemanager/daemon/AccessControlManager",
                                     "com.deepin.filemanager.daemon.AccessControlManager",
                                     QDBusConnection::systemBus(), this);

    auto response = deepin_systemInfo.call("FileManagerReply", QVariant::fromValue(policyState));
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        QVariantList value = response.arguments();
        if (!value.isEmpty()) {
            QVariant varVaule = value.first();
            if (!varVaule.toString().isEmpty()) {
                return true;
            }
        } else {
            return false;
        }

    } else {
        qDebug() << "value method called failed!";
        return false;
    }

    return false;
}

void VaultController::setVauleCurrentPageMark(VaultPageMark mark)
{
    m_recordVaultPageMark = mark;
}

VaultPageMark VaultController::getVaultCurrentPageMark()
{
    return m_recordVaultPageMark;
}

bool VaultController::isVaultVisiable()
{
    return m_vaultVisiable;
}

VaultController::~VaultController()
{
    if (d_ptr) {
        delete d_ptr;
        d_ptr = nullptr;
    }
}
