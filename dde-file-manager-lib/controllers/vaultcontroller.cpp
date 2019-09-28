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

#include "dfmevent.h"

#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>

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
    : DAbstractFileController(parent)
{
    prepareVaultDirs();
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

void VaultController::prepareVaultDirs()
{
    auto createIfNotExist = [](const QString & path){
        if (!QFile::exists(path)) {
            QDir().mkpath(path);
        }
    };
    static QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator();
    createIfNotExist(appDataLocation + "vault_encrypted");
    createIfNotExist(appDataLocation + "vault_unlocked");
}

bool VaultController::runVaultProcess(QStringList arguments, const DSecureString &stdinString)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) return false;

    QByteArray passwordByteArray = stdinString.toUtf8();
    passwordByteArray.append('\n');

    QProcess cryfsExec;
    cryfsExec.setEnvironment({"CRYFS_FRONTEND=noninteractive"});
    cryfsExec.start(cryfsBinary, arguments);
    cryfsExec.waitForStarted();
    cryfsExec.write(passwordByteArray);
    cryfsExec.waitForBytesWritten();
    cryfsExec.closeWriteChannel();
    cryfsExec.waitForFinished();
    cryfsExec.terminate();

    // about cryfs exitcode please refer to:
    // https://github.com/cryfs/cryfs/blob/develop/src/cryfs/impl/ErrorCodes.h
    return (cryfsExec.exitStatus() == QProcess::NormalExit && cryfsExec.exitCode() == 0);
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

QString VaultController::makeVaultLocalPath(QString path, QString base)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QDir::separator() + base + (path.startsWith('/') ? "" : "/") + path;
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

VaultController::VaultState VaultController::state()
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        return NotAvailable;
    }

    if (QFile::exists(makeVaultLocalPath("cryfs.config", "vault_encrypted"))) {
        QStorageInfo info(makeVaultLocalPath(""));
        if (info.isValid() && info.fileSystemType() == "fuse.cryfs") {
            return Unlocked;
        }

        return Encrypted;
    } else {
        return NotExisted;
    }
}

bool VaultController::createVault(const DSecureString &password)
{
    return VaultController::runVaultProcess({
                                                makeVaultLocalPath("", "vault_encrypted"),
                                                makeVaultLocalPath("", "vault_unlocked")
                                            }, password);
}

bool VaultController::unlockVault(const DSecureString &password)
{
    return VaultController::runVaultProcess({
                                                makeVaultLocalPath("", "vault_encrypted"),
                                                makeVaultLocalPath("", "vault_unlocked")
                                            }, password);
}

bool VaultController::lockVault()
{
    QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
    if (fusermountBinary.isEmpty()) return false;

    QProcess fusermountExec;
    fusermountExec.start(fusermountBinary, {"-u", makeVaultLocalPath("")});
    fusermountExec.waitForStarted();
    fusermountExec.waitForFinished();
    fusermountExec.terminate();

    return fusermountExec.exitStatus() == QProcess::NormalExit;
}
