/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "vaultlockmanager.h"

#include "appcontroller.h"
#include "singleton.h"
#include "dstorageinfo.h"

#include "tag/tagmanager.h"
#include "shutil/fileutils.h"
#include "usershare/shareinfo.h"
#include "app/define.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"

#include "dfmevent.h"

#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <unistd.h>

#include "vault/vaultlockmanager.h"

VaultController * VaultController::cryfs = nullptr;

class VaultControllerPrivate
{
public:
    explicit VaultControllerPrivate(VaultController * CryFs);

    CryFsHandle *m_cryFsHandle;

private:
    VaultController * q_ptr;
    Q_DECLARE_PUBLIC(VaultController)
};

VaultControllerPrivate::VaultControllerPrivate(VaultController *CryFs):q_ptr(CryFs)
{

}

class VaultDirIterator : public DDirIterator
{
public:
    VaultDirIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    QDirIterator *iterator;
};

VaultDirIterator::VaultDirIterator(const DUrl &url, const QStringList &nameFilters,
                                   QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
{
    iterator = new QDirIterator(VaultController::vaultToLocal(url), nameFilters, filter, flags);
}

DUrl VaultDirIterator::next()
{
    return VaultController::localToVault(iterator->next());
}

bool VaultDirIterator::hasNext() const
{
    return iterator->hasNext();
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
    : DAbstractFileController(parent),d_ptr(new VaultControllerPrivate(this))
{
    Q_D(VaultController);
    d->m_cryFsHandle = new CryFsHandle;
    connect(this, &VaultController::sigCreateVault, d->m_cryFsHandle, &CryFsHandle::createVault);
    connect(this, &VaultController::sigUnlockVault, d->m_cryFsHandle, &CryFsHandle::unlockVault);
    connect(this, &VaultController::sigLockVault, d->m_cryFsHandle, &CryFsHandle::lockVault);

    connect(d->m_cryFsHandle, &CryFsHandle::signalCreateVault, this, &VaultController::signalCreateVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalUnlockVault, this, &VaultController::signalUnlockVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalLockVault, this, &VaultController::signalLockVault);
    connect(d->m_cryFsHandle, &CryFsHandle::signalReadError, this, &VaultController::signalReadError);
    connect(d->m_cryFsHandle, &CryFsHandle::signalReadOutput, this, &VaultController::signalReadOutput);
}

VaultController *VaultController::getVaultController()
{
    if(!cryfs)
    {
        DUrl url(DFMVAULT_ROOT);
        url.setHost("files");
        url.setScheme(DFMVAULT_SCHEME);
        QList<DAbstractFileController *> vaultObjlist = DFileService::getHandlerTypeByUrl(url);
        if(vaultObjlist.size() > 0)
            cryfs = static_cast<VaultController *>(vaultObjlist.first());
    }
    return cryfs;
}

const DAbstractFileInfoPointer VaultController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new VaultFileInfo(event->url()));
}

const DDirIteratorPointer VaultController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    if (event->url().host() == "files") {
        return DDirIteratorPointer(new VaultDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
    }

    return nullptr;
}

DAbstractFileWatcher *VaultController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFileProxyWatcher(event->url(),
                                 new DFileWatcher(VaultController::vaultToLocal(event->url())),
                                 VaultController::localUrlToVault);
}

bool VaultController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), vaultToLocalUrl(event->url()));
}

bool VaultController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    DUrlList urlList1 = DFileService::instance()->moveToTrash(event->sender(), urlList);
    if(urlList == urlList1)
        return true;
    return false;
}

DUrlList VaultController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    return DFileService::instance()->moveToTrash(event->sender(), urlList);
}

bool VaultController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DUrlList urlList = vaultToLocalUrls(event->urlList());
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), urlList);
}

bool VaultController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    return DFileService::instance()->renameFile(event->sender(),
                                                vaultToLocalUrl(event->fromUrl()),
                                                vaultToLocalUrl(event->toUrl()));
}

bool VaultController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
{
    ShareInfo info;
    info.setPath(makeVaultLocalPath(event->name()));

    info.setShareName(event->name());
    info.setIsGuestOk(event->allowGuest());
    info.setIsWritable(event->isWritable());

    bool ret = userShareManager->addUserShare(info);

    return ret;
}

bool VaultController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    QString path = vaultToLocalUrl(event->fileUrl()).path();
    userShareManager->deleteUserShareByPath(path);

    return true;
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
    return DFileService::instance()->deleteFiles(nullptr, {DUrl::fromBookMarkFile(event->url(), QString())}, false);
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
        const QStringList &tags = TagManager::instance()->getTagsThroughFiles({durl});

        return tags.isEmpty() || TagManager::instance()->removeTagsOfFiles(tags, {durl});
    }

    return TagManager::instance()->makeFilesTags(taglist, {durl});
}

bool VaultController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    DUrl url = event->url();
    DUrl durl = vaultToLocalUrl(url);
    QList<QString> taglist = event->tags();
    return TagManager::instance()->removeTagsOfFiles(taglist, {durl});
}

QList<QString> VaultController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    DUrlList urllist = event->urlList();
    DUrlList tempList = vaultToLocalUrls(urllist);
    return TagManager::instance()->getTagsThroughFiles(tempList);
}

DUrl VaultController::makeVaultUrl(QString path, QString host)
{
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
    QString nextPath = localPath;
    int index = nextPath.indexOf("vault_unlocked");
    if (index == -1) {
        // fallback to vault file root dir.
        return VaultController::makeVaultUrl("/");
    }

    index += QString("vault_unlocked").length();

    return VaultController::makeVaultUrl(nextPath.mid(index));
}

QString VaultController::vaultToLocal(const DUrl &vaultUrl)
{
    Q_ASSERT(vaultUrl.scheme() == DFMVAULT_SCHEME);
    return makeVaultLocalPath(vaultUrl.path());
}

DUrl VaultController::vaultToLocalUrl(const DUrl &vaultUrl)
{
    Q_ASSERT(vaultUrl.scheme() == DFMVAULT_SCHEME);
    if (vaultUrl.scheme() != DFMVAULT_SCHEME) return vaultUrl;
    return DUrl::fromLocalFile(vaultToLocal(vaultUrl));
}

DUrlList VaultController::vaultToLocalUrls(DUrlList vaultUrls)
{
    for (DUrl &url : vaultUrls) {
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
        url = vaultToLocalUrl(url);
    }

    return vaultUrls;
}

bool VaultController::checkAuthentication()
{
    QString standOutput("");
    bool res = runCmd("id -un", standOutput);
    if (res && standOutput.trimmed() == "root"){
        return true;
    }

    return runCmd("pkexec deepin-devicemanager-authenticateProxy", standOutput);
}

VaultController::VaultState VaultController::state(QString lockBaseDir)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        return NotAvailable;
    }

    if(lockBaseDir.isEmpty())
    {
        lockBaseDir = makeVaultLocalPath("cryfs.config", "vault_encrypted");
    }
    else
    {
        if(lockBaseDir.endsWith("/"))
            lockBaseDir += "cryfs.config";
        else
            lockBaseDir += "/cryfs.config";
    }
    if (QFile::exists(lockBaseDir))
    {
        QStorageInfo info(makeVaultLocalPath(""));
        QString temp = info.fileSystemType();
        if (info.isValid() && temp == "fuse.cryfs")
        {
            return Unlocked;
        }

        return Encrypted;
    } else {
        return NotExisted;
    }
}

void VaultController::createVault(const DSecureString & passWord, QString lockBaseDir, QString unlockFileDir)
{
    auto createIfNotExist = [](const QString & path){
        if (!QFile::exists(path)) {
            QDir().mkpath(path);
        }
    };

    if(lockBaseDir.isEmpty() || unlockFileDir.isEmpty())
    {
        if(state() != NotExisted)
        {
            emit signalCreateVault(static_cast<int>(ErrorCode::EncryptedExist));
            return;
        }

        createIfNotExist(makeVaultLocalPath("", "vault_encrypted"));
        createIfNotExist(makeVaultLocalPath("", "vault_unlocked"));

        emit sigCreateVault(makeVaultLocalPath("", "vault_encrypted"),
                            makeVaultLocalPath("", "vault_unlocked"),
                            passWord);
    }
    else
    {
        if(state(lockBaseDir) != NotExisted)
        {
            emit signalCreateVault(static_cast<int>(ErrorCode::EncryptedExist));
            return;
        }

        createIfNotExist(lockBaseDir);
        createIfNotExist(unlockFileDir);
        emit sigCreateVault(lockBaseDir, unlockFileDir, passWord);
    }
}

void VaultController::unlockVault(const DSecureString &passWord, QString lockBaseDir, QString unlockFileDir)
{
    if(lockBaseDir.isEmpty() || unlockFileDir.isEmpty())
    {
        if(state() != Encrypted)
        {
            emit signalUnlockVault(static_cast<int>(ErrorCode::MountpointNotEmpty));
            return;
        }

        emit sigUnlockVault(makeVaultLocalPath("", "vault_encrypted"),
                            makeVaultLocalPath("", "vault_unlocked"),
                            passWord);
    }
    else
    {
        if(state(lockBaseDir) != Encrypted)
        {
            emit signalUnlockVault(static_cast<int>(ErrorCode::MountpointNotEmpty));
            return;
        }
        emit sigUnlockVault(lockBaseDir, unlockFileDir, passWord);
    }
}

void VaultController::lockVault(QString lockBaseDir, QString unlockFileDir)
{
    if(lockBaseDir.isEmpty() || unlockFileDir.isEmpty())
    {
        if(state() != Unlocked)
        {
            emit signalLockVault(static_cast<int>(ErrorCode::MountdirEncrypted));
            return;
        }
        emit sigLockVault(makeVaultLocalPath("", "vault_unlocked"));
    }
    else
    {
        if(state(lockBaseDir) != Unlocked)
        {
            emit signalLockVault(static_cast<int>(ErrorCode::MountdirEncrypted));
            return;
        }
        emit sigLockVault(unlockFileDir);
    }
}

QString VaultController::makeVaultLocalPath(QString path, QString base)
{
    if(base.isEmpty())
    {
        base = "vault_unlocked";
    }
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QDir::separator() + base + (path.startsWith('/') ? "" : "/") + path;
}

QString VaultController::vaultLockPath()
{
    return makeVaultLocalPath("", "vault_encrypted");
}

QString VaultController::vaultUnlockPath()
{
    return makeVaultLocalPath("", "vault_unlocked");
}

bool VaultController::runCmd(const QString &cmd, QString &standOutput)
{
    QProcess process_;
    int msecs = 10000;
    if (cmd.startsWith("pkexec deepin-vaultRemove-authenticateProxy") ) {
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    standOutput = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if ( cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126) ) {
        //dError( "Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");

        if (cmd.contains("whoami")) {
            //if(exitCode == 126)
            //{
            //DMessageBox::critical(nullptr, "", tr("Password Error!" ));
            //}

            //exit(-1);
        }

        return false;
    }

    if (res == false) {
        //dError( "Run \'" + cmd + "\' failed\n" );
    }

    return res;
}


