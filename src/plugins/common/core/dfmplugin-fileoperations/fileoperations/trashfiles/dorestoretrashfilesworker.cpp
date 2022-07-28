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
#include "dorestoretrashfilesworker.h"
#include "fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/decorator/decoratorfile.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>

#include <QUrl>
#include <QDebug>
#include <QMutex>
#include <QSettings>
#include <QStorageInfo>

DPFILEOPERATIONS_USE_NAMESPACE
DoRestoreTrashFilesWorker::DoRestoreTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kRestoreType;
}

DoRestoreTrashFilesWorker::~DoRestoreTrashFilesWorker()
{
    stop();
}

bool DoRestoreTrashFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    doRestoreTrashFiles();
    // 完成
    endWork();

    return true;
}

bool DoRestoreTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();

    if (sourceUrls.count() == 0) {
        qWarning() << "sources files list is empty!";
        return false;
    }

    QString path = sourceUrls.first().path();
    if (path.endsWith("/"))
        path.chop(1);

    if (sourceUrls.count() == 1 && path == StandardPaths::location(StandardPaths::kTrashFilesPath)) {
        FileOperationsUtils::getDirFiles(sourceUrls.first(), allFilesList);
        sourceFilesCount = allFilesList.size();
    }

    return true;
}

bool DoRestoreTrashFilesWorker::initArgs()
{
    trashStorageInfo.reset(new QStorageInfo(StandardPaths::location(StandardPaths::kTrashFilesPath)));
    trashInfoPath = StandardPaths::location(StandardPaths::kTrashInfosPath);
    trashInfoPath = trashInfoPath.endsWith("/") ? trashInfoPath : trashInfoPath + "/";
    completeTargetFiles.clear();
    isConvert = jobFlags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kRevocation);
    return AbstractWorker::initArgs();
}
/*!
 * \brief DoRestoreTrashFilesWorker::doRestoreTrashFiles Performing a recycle bin restore
 * \return Is the recycle bin restore successful
 */
bool DoRestoreTrashFilesWorker::doRestoreTrashFiles()
{
    //获取当前的
    bool result = false;
    // 总大小使用源文件个数
    QList<QUrl> urlsSource = sourceUrls;
    if (!allFilesList.empty())
        urlsSource = allFilesList;
    for (const auto &url : urlsSource) {
        if (!stateCheck())
            return false;

        // 获取回收站文件的原路径

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                compeleteFilesCount++;
                continue;
            }
        }

        QUrl restoreFileUrl;

        if (!getRestoreFileUrl(fileInfo, restoreFileUrl, &result)) {
            if (result) {
                compeleteFilesCount++;
                continue;
            } else {
                return false;
            }
        }

        const auto &restoreInfo = InfoFactory::create<AbstractFileInfo>(restoreFileUrl);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, restoreFileUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                compeleteFilesCount++;
                continue;
            }
        }

        AbstractFileInfoPointer targetInfo = nullptr;
        if (!createParentDir(fileInfo, restoreInfo, targetInfo, &result)) {
            if (result) {
                compeleteFilesCount++;
                continue;
            } else {
                return false;
            }
        }

        bool ok = false;

        QString trashInfoFile(trashInfoPath + fileInfo->fileName() + ".trashinfo");
        DecoratorFile file(trashInfoFile);
        QString trashInfoCache;
        if (file.exists())
            trashInfoCache = file.readAll();

        if (fileInfo->isSymLink()) {
            ok = handleSymlinkFile(fileInfo, restoreInfo);
        } else {
            ok = handleRestoreTrash(fileInfo, restoreInfo, targetInfo);
            if (!ok)
                emit requestShowTipsDialog(DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kRestoreFailed, { url });
        }

        if (!ok) {
            return false;
        }

        // save info
        {
            if (!completeSourceFiles.contains(url)) {
                completeSourceFiles.append(url);
                completeCustomInfos.append(trashInfoCache);
            }
            if (!completeTargetFiles.contains(restoreInfo->url()))
                completeTargetFiles.append(restoreInfo->url());
        }
    }

    return true;
}
/*!
 * \brief DoRestoreTrashFilesWorker::getRestoreFileUrl Get the original URL of the restored file
 * at the rediscovery station
 * \param trashFileInfo Information about files in the recycle bin
 * \param restoreUrl Output parameter: the original URL restored by the recycle bin
 * \param result Output parameters: execution results
 * \return Is it successful
 */
bool DoRestoreTrashFilesWorker::getRestoreFileUrl(const AbstractFileInfoPointer &trashFileInfo,
                                                  QUrl &restoreUrl, bool *result)
{
    restoreUrl.clear();
    const QString &fileBaseName = trashFileInfo->fileName();

    QString location(trashInfoPath + fileBaseName + ".trashinfo");
    if (QFile::exists(location)) {
        QSettings setting(location, QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        QString originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray());

        restoreUrl = QUrl::fromLocalFile(originalFilePath);
    }

    if (!restoreUrl.isValid()) {
        AbstractJobHandler::SupportAction action = doHandleErrorAndWait(trashFileInfo->url(), restoreUrl, AbstractJobHandler::JobErrorType::kGetRestorePathError);
        if (result)
            *result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoRestoreTrashFilesWorker::handleSymlinkFile Process linked file recycle bin restore
 * \param trashInfo File information in Recycle Bin
 * \param restoreInfo the file information restored at the originating station
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::handleSymlinkFile(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo)
{
    //如果文件存在，弹出窗口提示，替换还是跳过。跳过也会清理掉当前的trashinfo和源文件
    const QUrl &fromUrl = trashInfo->url();
    const QUrl &toUrl = restoreInfo->url();

    emitCurrentTaskNotify(fromUrl, toUrl);

    QDir parentDir(UrlRoute::urlParent(toUrl).path());
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    if (restoreInfo->exists()) {
        action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kFileExistsError);
    } else {
        do {
            QFile targetfile(trashInfo->symLinkTarget());
            if (!targetfile.link(restoreInfo->filePath()))
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kSymlinkError, targetfile.errorString());
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    }

    // clear tashfileinfo
    return clearTrashFile(fromUrl, toUrl, trashInfo) && (action == AbstractJobHandler::SupportAction::kNoAction || action == AbstractJobHandler::SupportAction::kSkipAction || action == AbstractJobHandler::SupportAction::kReplaceAction);
}
/*!
 * \brief DoRestoreTrashFilesWorker::handleRestoreTrash Process file recycle bin restore
 * \param trashInfo File information in Recycle Bin
 * \param restoreInfo the file information restored at the originating station
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::handleRestoreTrash(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo, const AbstractFileInfoPointer &targetInfo)
{
    //执行dorename，失败执行拷贝，再执行清理掉当前的trashinfo和源文件
    const QUrl &fromUrl = trashInfo->url();
    QUrl toUrl = restoreInfo->url();
    emitCurrentTaskNotify(fromUrl, toUrl);
    compeleteFilesCount++;

    bool ok = false;
    AbstractFileInfoPointer newTargetInfo(nullptr);
    if (!doCheckFile(trashInfo, targetInfo, restoreInfo->fileName(), newTargetInfo, &ok))
        return ok;

    toUrl = newTargetInfo->url();

    if (handler->renameFile(fromUrl, toUrl))
        return clearTrashFile(fromUrl, toUrl, trashInfo, true);

    return false;
}
/*!
 * \brief DoRestoreTrashFilesWorker::clearTrashFile
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param trashInfo File information in Recycle Bin
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::clearTrashFile(const QUrl &fromUrl, const QUrl &toUrl, const AbstractFileInfoPointer &trashInfo, const bool isSourceDel)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QString location(trashInfoPath + trashInfo->fileName() + ".trashinfo");
    bool resultFile = isSourceDel;
    bool resultInfo = false;
    do {
        if (!resultFile)
            resultFile = handler->deleteFile(trashInfo->url());
        if (!resultInfo)
            resultInfo = handler->deleteFile(QUrl::fromLocalFile(location));
        if (!resultInfo || !resultFile)
            action = doHandleErrorAndWait(fromUrl, toUrl,
                                          AbstractJobHandler::JobErrorType::kDeleteTrashFileError, handler->errorString());
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    return action == AbstractJobHandler::SupportAction::kNoAction || AbstractJobHandler::SupportAction::kSkipAction == action;
}
/*!
 * \brief DoRestoreTrashFilesWorker::checkDiskSpaceAvailable Check if there is free space
 * on the disk where the target file is located
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param result Output parameters: operation results
 * \return Is there any extra space
 */
bool DoRestoreTrashFilesWorker::checkDiskSpaceAvailable(const QUrl &fromUrl, const QUrl &toUrl, bool *result)
{

    QSharedPointer<StorageInfo> restoreStorage(new StorageInfo(toUrl.path()));
    return FileOperateBaseWorker::checkDiskSpaceAvailable(fromUrl, toUrl, restoreStorage, result);
}
/*!
 * \brief DoRestoreTrashFilesWorker::doCopyAndClearTrashFile Copy and clean recycle bin files
 * \param trashInfo File information in Recycle Bin
 * \param restoreInfo the file information restored at the originating station
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::doCopyAndClearTrashFile(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo)
{
    bool result = false;
    // 判断目标文件是否存在，跳过，和替换操作
    const QUrl &trashUrl = trashInfo->url();
    const QUrl &restoreUrl = restoreInfo->url();
    emitCurrentTaskNotify(trashUrl, restoreUrl);

    if (restoreInfo->exists()) {
        AbstractJobHandler::SupportAction actionForExist { AbstractJobHandler::SupportAction::kNoAction };
        if (trashInfo->isFile()) {
            actionForExist = doHandleErrorAndWait(trashUrl, restoreUrl, AbstractJobHandler::JobErrorType::kFileExistsError);
        } else {
            actionForExist = doHandleErrorAndWait(trashUrl, restoreUrl, AbstractJobHandler::JobErrorType::kDirectoryExistsError);
        }

        if (actionForExist == AbstractJobHandler::SupportAction::kSkipAction) {
            compeleteFilesCount++;
            return true;
        }
    }

    if (trashInfo->isFile()) {
        if (!doCopyFilePractically(trashInfo, restoreInfo, &result))
            return result;
    } else {
        if (!checkAndCopyDir(trashInfo, restoreInfo, &result))
            return result;
    }

    compeleteFilesCount++;

    return clearTrashFile(trashUrl, restoreUrl, trashInfo, false);
}

bool DoRestoreTrashFilesWorker::createParentDir(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo,
                                                AbstractFileInfoPointer &targetFileInfo, bool *result)
{
    const QUrl &fromUrl = trashInfo->url();
    const QUrl &toUrl = restoreInfo->url();
    const QUrl &parentUrl = UrlRoute::urlParent(toUrl);
    if (!parentUrl.isValid())
        return false;
    targetFileInfo.reset(nullptr);
    targetFileInfo = InfoFactory::create<AbstractFileInfo>(parentUrl);
    if (!targetFileInfo)
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    if (!targetFileInfo->exists()) {
        do {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            if (!fileHandler.mkdir(parentUrl))
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, AbstractJobHandler::JobErrorType::kCreateParentDirError);
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        if (action != AbstractJobHandler::SupportAction::kNoAction) {
            if (result)
                *result = action == AbstractJobHandler::SupportAction::kSkipAction;
            return false;
        }
    }

    return true;
}
