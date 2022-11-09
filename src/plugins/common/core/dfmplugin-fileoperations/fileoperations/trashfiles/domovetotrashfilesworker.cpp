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
#include "fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>
#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>
#include <QtGlobal>
#include <QCryptographicHash>
#include <QStorageInfo>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

USING_IO_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE
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

    // check progress notify type
    determineCountProcessType();

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
    targetUrl = FileUtils::trashRootUrl();
    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doMoveToTrash do move to trash
 * \return move to trash success
 */
bool DoMoveToTrashFilesWorker::doMoveToTrash()
{
    bool result = false;
    // 总大小使用源文件个数
    for (const auto &url : sourceUrls) {
        if (!stateCheck())
            return false;

        if (FileUtils::isTrashFile(url)) {
            completeFilesCount++;
            completeSourceFiles.append(url);
            continue;
        }

        // url是否可以删除 canrename
        if (!isCanMoveToTrash(url, &result)) {
            if (result) {
                completeFilesCount++;
                completeSourceFiles.append(url);
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

        emitCurrentTaskNotify(url, targetUrl);

        QByteArray tagData;
        auto tags = dpfSlotChannel->push("dfmplugin_tag", "slot_GetTags", QUrl::fromUserInput(url.path())).toStringList();
        if (!tags.isEmpty())
            tagData.append("TagNameList=").append(tags.join(',')).append('\n');

        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.trashFile(url);
        if (trashSucc) {
            // update tag info to trash info
            writeTagInfo(url, tagData);

            completeFilesCount++;
            emitProgressChangedNotify(completeFilesCount);
            completeSourceFiles.append(url);
            continue;
        }
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

bool DoMoveToTrashFilesWorker::writeTagInfo(const QUrl &url, const QByteArray &data)
{
    DecoratorFileEnumerator enumerator(FileUtils::trashRootUrl());
    if (!enumerator.isValid())
        return false;

    QUrl targetInfo;
    while (enumerator.hasNext()) {
        const QUrl &urlNext = enumerator.next();
        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(urlNext);
        if (!fileInfo)
            continue;
        const QUrl &originUrl = fileInfo->originalUrl();
        if (UniversalUtils::urlEquals(url, originUrl)) {
            const QUrl &fileTargetUrl = fileInfo->redirectedFileUrl();
            QString urlTemp = fileTargetUrl.toString() + ".trashinfo";
            urlTemp.replace("/files/", "/info/");
            targetInfo = urlTemp;

            completeTargetFiles.append(urlNext);
        }
    }

    if (!data.isEmpty()) {
        DecoratorFile file(targetInfo);
        const qint64 &size = file.writeAll(data, DFMIO::DFile::OpenFlag::kAppend);
        return size > 0;
    }
    return false;
}
