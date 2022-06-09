/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "domovetotrashfilesworker.h"
#include "services/common/fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>
#include <QWaitCondition>
#include <QtGlobal>
#include <QCryptographicHash>
#include <QStorageInfo>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

USING_IO_NAMESPACE
DSC_USE_NAMESPACE
DoMoveToTrashFilesWorker::DoMoveToTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kMoveToTrashType;
}

DoMoveToTrashFilesWorker::~DoMoveToTrashFilesWorker()
{
    stop();
}
/*!
 * \brief DoMoveToTrashFilesWorker::doWork the thread move to trash work function
 * \return move to trash is successed
 */
bool DoMoveToTrashFilesWorker::doWork()
{
    if (!AbstractWorker::doWork())
        return false;

    doMoveToTrash();

    endWork();

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::statisticsFilesSize init target arguments
 * \return
 */
bool DoMoveToTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();
    targetUrl = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashFilesPath).endsWith("/") ? StandardPaths::location(StandardPaths::kTrashFilesPath) : StandardPaths::location(StandardPaths::kTrashFilesPath) + "/");
    trashLocalDir = QString("%1/.local/share/Trash").arg(QDir::homePath());
    targetStorageInfo.reset(new StorageInfo(trashLocalDir));

    targetInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doMoveToTrash do move to trash
 * \return move to trash success
 */
bool DoMoveToTrashFilesWorker::doMoveToTrash()
{
    if (!checkTrashDirIsReady())
        return false;

    bool result = false;
    // 总大小使用源文件个数
    for (const auto &url : sourceUrls) {
        if (!stateCheck())
            return false;

        if (url.path().startsWith(targetUrl.path())) {
            completeFilesCount++;
            continue;
        }

        // url是否可以删除 canrename
        if (!isCanMoveToTrash(url, &result)) {
            if (result) {
                completeFilesCount++;
                continue;
            }

            return false;
        }

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        isInSameDisk(fileInfo);

        bool ok = false;

        if (fileInfo->isSymLink()) {
            ok = handleSymlinkFile(fileInfo);
        } else {
            ok = handleMoveToTrash(fileInfo);
        }

        if (!ok)
            return false;
    }
    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::checkTrashDirIsReady check trash dir is exit or ready
 * \return
 */
bool DoMoveToTrashFilesWorker::checkTrashDirIsReady()
{
    QDir trashDir;

    if (!trashDir.mkpath(StandardPaths::location(StandardPaths::kTrashFilesPath))) {
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kMakeStandardTrashError);
        qWarning() << " mk " << StandardPaths::location(StandardPaths::kTrashInfosPath) << "failed!";

        return false;
    }

    if (!trashDir.mkpath(StandardPaths::location(StandardPaths::kTrashInfosPath))) {
        doHandleErrorAndWait(sourceUrls.first(), targetUrl, AbstractJobHandler::JobErrorType::kMakeStandardTrashError);
        qWarning() << " mk " << StandardPaths::location(StandardPaths::kTrashInfosPath) << "failed!";

        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::isCanMoveToTrash loop to check the source file can move to trash
 * \param url the source file url
 * \param result Output parameters, is skip this file
 * \return can move to trash
 */
bool DoMoveToTrashFilesWorker::isCanMoveToTrash(const QUrl &url, bool *result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    do {
        if (!canWriteFile(url))
            // pause and emit error msg
            action = doHandleErrorAndWait(url, targetUrl, AbstractJobHandler::JobErrorType::kPermissionDeniedError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::handleSymlinkFile Process linked files, move to the recycle bin,
 * create new linked files, and delete source linked files
 * \param File information of link file
 * \return Process successed
 */
bool DoMoveToTrashFilesWorker::handleSymlinkFile(const AbstractFileInfoPointer &fileInfo)
{
    if (!stateCheck())
        return false;

    const QUrl &fromUrl = fileInfo->url();

    QFileInfo fromInfo(fromUrl.path());
    QString srcFileName = fromInfo.fileName();
    QString srcPath = fileInfo->path();
    QString targetPath;

    emitCurrentTaskNotify(fileInfo->url(), targetUrl);

    bool result = false;
    if (!writeTrashInfo(fileInfo, targetPath, &result)) {
        return result;
    }
    QFile targetFile(fileInfo->symLinkTarget());

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        if (!targetFile.link(targetPath))
            // pause and emit error msg
            action = doHandleErrorAndWait(fromUrl, targetUrl, AbstractJobHandler::JobErrorType::kSymlinkError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        return action == AbstractJobHandler::SupportAction::kSkipAction;
    }

    if (!isConvert) {
        completeSourceFiles.append(fileInfo->url());
        completeSourceFiles.append(QUrl::fromLocalFile(targetPath));
    }

    handler->deleteFile(fileInfo->url());

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::handleMoveToTrash Processing ordinary files and moving them to the recycle bin
 * \param fileInfo File information of source file
 * \return Is the process successful
 */
bool DoMoveToTrashFilesWorker::handleMoveToTrash(const AbstractFileInfoPointer &fileInfo)
{
    QString targetPath;
    bool result = false;
    if (!writeTrashInfo(fileInfo, targetPath, &result)) {
        return result;
    }

    const QUrl &sourceUrl = fileInfo->url();
    const QUrl &toUrl = QUrl::fromLocalFile(targetPath);

    emitCurrentTaskNotify(fileInfo->url(), toUrl);

    if (checkFileOutOfLimit(fileInfo)) {
        qWarning() << "move to trash big file, use delete way, url: " << sourceUrl;

        // todo lanxs, need tips dialog
        completeFilesCount++;
        // ToDo::大于1G，执行彻底删除代码
        if (fileInfo->isFile() || fileInfo->isSymLink())
            return deleteFile(sourceUrl, QUrl(), &result);
        else
            return deleteDir(sourceUrl, QUrl(), &result);
    }

    // ToDo::判断是否同盘，是就直接rename
    if (isSameDisk == 1) {
        qDebug() << "move to trash is on same disk, from url: " << fileInfo->url() << " to url: " << toUrl;
        AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

        do {
            if (!handler->renameFile(fileInfo->url(), QUrl::fromLocalFile(targetPath))) {
                action = doHandleErrorAndWait(sourceUrl, QUrl::fromLocalFile(targetPath), AbstractJobHandler::JobErrorType::kRenameError, handler->errorString());
            }
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        completeFilesCount++;

        if (!isConvert && action == AbstractJobHandler::SupportAction::kNoAction) {
            completeSourceFiles.append(fileInfo->url());
            completeTargetFiles.append(QUrl::fromLocalFile(targetPath));
        }

        if (action == AbstractJobHandler::SupportAction::kSkipAction
            || action == AbstractJobHandler::SupportAction::kNoAction
            || action == AbstractJobHandler::SupportAction::kCancelAction)
            return true;
    }

    const QUrl &parentUrl = DFMUtils::directParentUrl(toUrl);
    if (!parentUrl.isValid())
        return false;

    const auto &targetPathInfo = InfoFactory::create<AbstractFileInfo>(parentUrl);
    if (!targetPathInfo) {
        // pause and emit error msg
        return AbstractJobHandler::SupportAction::kSkipAction == doHandleErrorAndWait(sourceUrl, targetPathInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError);
    }
    // 检查磁盘空间是否不足
    if (!checkDiskSpaceAvailable(sourceUrl, parentUrl, targetStorageInfo, &result))
        return result;

    // 拷贝并删除文件
    qDebug() << "rename failed, use copy and delete way, from url :" << fileInfo->url() << " to url: " << targetPathInfo->url();

    AbstractFileInfoPointer toInfo = InfoFactory::create<AbstractFileInfo>(toUrl);
    if (!toInfo)
        return false;
    return copyAndDeleteFile(fileInfo, targetPathInfo, toInfo, &result);
}
/*!
 * \brief DoMoveToTrashFilesWorker::checkFileOutOfLimit Check whether the file size exceeds the limit value
 * \param fileInfo File information of source file
 * \return Have you manipulated the limit value
 */
bool DoMoveToTrashFilesWorker::checkFileOutOfLimit(const AbstractFileInfoPointer &fileInfo)
{
    return FileOperationsUtils::isFilesSizeOutLimit(fileInfo->url(), 1024 * 1024 * 1024);
}

bool DoMoveToTrashFilesWorker::writeTrashInfo(const AbstractFileInfoPointer &fileInfo, QString &targetPath, bool *result)
{
    if (!stateCheck())
        return false;

    QString path = targetUrl.path();
    QString baseName = getNotExistsTrashFileName(fileInfo->fileName());
    QString newName = path + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    qDebug() << " writeTrashInfo " << fileInfo->url();
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        if (!doWriteTrashInfo(baseName, fileInfo->filePath(), delTime))
            // pause and emit error msg
            action = doHandleErrorAndWait(fileInfo->url(), targetUrl, AbstractJobHandler::JobErrorType::kPermissionDeniedError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    targetPath = newName;

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::getNotExistsTrashFileName Gets the name of a nonexistent recycle bin file
 * \param fileName the name of source file
 * \return the name of a nonexistent recycle bin file
 */
QString DoMoveToTrashFilesWorker::getNotExistsTrashFileName(const QString &fileName)
{
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('/');

    if (index >= 0)
        name = name.mid(index + 1);

    index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0)
        suffix = name.mid(index);

    if (suffix.size() > 200)
        suffix = suffix.left(200);

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());

    QString trashpath = StandardPaths::location(StandardPaths::kTrashFilesPath) + "/";

    while (true) {
        QFileInfo info(trashpath + name + suffix);
        // QFile::exists ==> If the file is a symlink that points to a non-existing file, false is returned.
        if (!info.isSymLink() && !info.exists()) {
            break;
        }

        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}
/*!
 * \brief DoMoveToTrashFilesWorker::doWriteTrashInfo Write information about files moved to the recycle bin
 * \param fileBaseName the base Name of the file moved to the recycle bin
 * \param path The path to the recycle bin file
 * \param time Time the file was moved to the recycle bin
 * \return Whether the write is complete
 */
bool DoMoveToTrashFilesWorker::doWriteTrashInfo(const QString &fileBaseName, const QString &path, const QString &time)
{
    QFile metadata(trashLocalDir + "/info/" + fileBaseName + ".trashinfo");

    if (!metadata.open(QIODevice::WriteOnly)) {
        qDebug() << metadata.fileName() << "file open error:" << metadata.errorString();

        return false;
    }

    QByteArray data;

    data.append("[Trash Info]\n");
    data.append("Path=").append(path.toUtf8().toPercentEncoding("/")).append("\n");
    data.append("DeletionDate=").append(time).append("\n");

    // save the file tag info
    const QStringList tag_name_list;

    qint64 size = metadata.write(data);

    metadata.close();

    if (size < 0) {
        qDebug() << "write file " << metadata.fileName() << "error:" << metadata.errorString();
    }

    return size > 0;
}
/*!
 * \brief DoMoveToTrashFilesWorker::getIsInSameDisk Check that the
 * source file and the recycle bin directory are on the same disk
 * \param fileInfo File information of source file
 */
void DoMoveToTrashFilesWorker::isInSameDisk(const AbstractFileInfoPointer &fileInfo)
{
    if (isSameDisk > -1)
        return;

    const QStorageInfo &sourceStorage = fileInfo->isSymLink() ? QStorageInfo(fileInfo->absolutePath())
                                                              : QStorageInfo(fileInfo->absoluteFilePath());

    isSameDisk = sourceStorage.device() == targetStorageInfo->device();
}
