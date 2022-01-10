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
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>

#include <QUrl>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <QSettings>
#include <QStorageInfo>

DSC_USE_NAMESPACE
DoRestoreTrashFilesWorker::DoRestoreTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
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

    // ToDo::执行从回收站还原的业务逻辑
    doRestoreTrashFiles();
    // 完成
    endWork();

    return true;
}

bool DoRestoreTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();
    return true;
}

bool DoRestoreTrashFilesWorker::initArgs()
{
    trashStorageInfo.reset(new QStorageInfo(StandardPaths::location(StandardPaths::kTrashFilesPath)));
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
    for (const auto &url : sourceUrls) {
        if (!stateCheck())
            return false;

        // 获取回收站文件的原路径

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), url, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                compeleteFilesCount++;
                continue;
            }
        }

        QUrl restoreFileUrl;

        if (!getRestoreFileUrl(fileInfo, restoreFileUrl, result)) {
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
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, restoreFileUrl, restoreFileUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                compeleteFilesCount++;
                continue;
            }
        }

        bool ok = false;

        if (fileInfo->isSymLink()) {
            ok = handleSymlinkFile(fileInfo, restoreInfo);
        } else {
            ok = handleRestoreTrash(fileInfo, restoreInfo);
        }

        if (!ok)
            return false;
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
                                                  QUrl &restoreUrl, bool &result)
{
    restoreUrl.clear();
    const QString &fileBaseName = trashFileInfo->baseName();

    QString location(StandardPaths::location(StandardPaths::kTrashInfosPath) + fileBaseName + ".trashinfo");
    if (QFile::exists(location)) {
        QSettings setting(location, QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        QString originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray());

        restoreUrl = QUrl::fromLocalFile(originalFilePath);
    }

    if (!restoreUrl.isValid()) {
        result = AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(trashFileInfo->url(), restoreUrl, restoreUrl, AbstractJobHandler::JobErrorType::kGetRestorePathError);
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

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    if (restoreInfo->exists()) {
        action = doHandleErrorAndWait(fromUrl, toUrl, toUrl, AbstractJobHandler::JobErrorType::kFileExistsError);
    } else {
        do {
            QFile targetFile(trashInfo->symLinkTarget());
            if (!targetFile.link(restoreInfo->filePath()))
                // pause and emit error msg
                action = doHandleErrorAndWait(fromUrl, toUrl, fromUrl, AbstractJobHandler::JobErrorType::kSymlinkError, targetFile.errorString());
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    }
    // 清理tashfileinfo

    return clearTrashFile(fromUrl, toUrl, trashInfo) && (action == AbstractJobHandler::SupportAction::kNoAction || action == AbstractJobHandler::SupportAction::kSkipAction || action == AbstractJobHandler::SupportAction::kReplaceAction);
}
/*!
 * \brief DoRestoreTrashFilesWorker::handleRestoreTrash Process file recycle bin restore
 * \param trashInfo File information in Recycle Bin
 * \param restoreInfo the file information restored at the originating station
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::handleRestoreTrash(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo)
{
    //执行dorename，失败执行拷贝，再执行清理掉当前的trashinfo和源文件
    const QUrl &fromUrl = trashInfo->url();
    const QUrl &toUrl = restoreInfo->url();
    emitCurrentTaskNotify(fromUrl, toUrl);

    if (restoreInfo->exists()) {
        AbstractJobHandler::SupportAction actionForExists { AbstractJobHandler::SupportAction::kNoAction };
        if (trashInfo->isFile()) {
            actionForExists = doHandleErrorAndWait(fromUrl, toUrl, toUrl, AbstractJobHandler::JobErrorType::kFileExistsError);
        } else {
            actionForExists = doHandleErrorAndWait(fromUrl, toUrl, toUrl, AbstractJobHandler::JobErrorType::kDirectoryExistsError);
        }

        if (actionForExists == AbstractJobHandler::SupportAction::kSkipAction) {
            compeleteFilesCount++;
            return true;
        }
    }

    AbstractJobHandler::SupportAction actionForRename = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!handler->renameFile(fromUrl, toUrl)) {
            actionForRename = doHandleErrorAndWait(fromUrl, toUrl, fromUrl, AbstractJobHandler::JobErrorType::kRenameError, handler->errorString());
        }
    } while (isStopped() && actionForRename == AbstractJobHandler::SupportAction::kRetryAction);

    compeleteFilesCount++;

    if (actionForRename == AbstractJobHandler::SupportAction::kSkipAction || actionForRename == AbstractJobHandler::SupportAction::kNoAction)
        return true;

    bool result = false;
    // 检查磁盘空间是否不足
    if (!checkDiskSpaceAvailable(fromUrl, toUrl, &result))
        return result;

    return doCopyAndClearTrashFile(trashInfo, restoreInfo);
}
/*!
 * \brief DoRestoreTrashFilesWorker::clearTrashFile
 * \param fromUrl URL of the source file
 * \param toUrl Destination URL
 * \param trashInfo File information in Recycle Bin
 * \return Is the execution successful
 */
bool DoRestoreTrashFilesWorker::clearTrashFile(const QUrl &fromUrl, const QUrl &toUrl, const AbstractFileInfoPointer &trashInfo)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QString location(StandardPaths::location(StandardPaths::kTrashInfosPath) + trashInfo->baseName() + ".trashinfo");
    bool resultFile = false;
    bool resultInfo = false;
    do {
        if (!resultFile)
            resultFile = handler->deleteFile(trashInfo->url());
        if (!resultInfo)
            resultInfo = handler->deleteFile(QUrl::fromLocalFile(location));
        if (!resultInfo || !resultFile)
            action = doHandleErrorAndWait(fromUrl, toUrl,
                                          !resultInfo ? trashInfo->url() : QUrl::fromLocalFile(location),
                                          AbstractJobHandler::JobErrorType::kDeleteTrashFileError, handler->errorString());
    } while (isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

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

    QSharedPointer<QStorageInfo> restoreStorage(new QStorageInfo(toUrl.path()));

    FileOperateBaseWorker::checkDiskSpaceAvailable(fromUrl, toUrl, restoreStorage, result);

    return true;
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
            actionForExist = doHandleErrorAndWait(trashUrl, restoreUrl, restoreUrl, AbstractJobHandler::JobErrorType::kFileExistsError);
        } else {
            actionForExist = doHandleErrorAndWait(trashUrl, restoreUrl, restoreUrl, AbstractJobHandler::JobErrorType::kDirectoryExistsError);
        }

        if (actionForExist == AbstractJobHandler::SupportAction::kSkipAction) {
            compeleteFilesCount++;
            return true;
        }
    }

    if (trashInfo->isFile()) {
        if (!copyFile(trashInfo, restoreInfo, &result))
            return result;

    } else {
        if (!copyDir(trashInfo, restoreInfo, &result))
            return result;
    }

    return clearTrashFile(trashUrl, restoreUrl, trashInfo);
}
