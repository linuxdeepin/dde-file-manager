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
#include "dfm-base/base/device/deviceutils.h"

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
    fstabMap = DeviceUtils::fstabBindInfo();
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
        QUrl urlSource = url;
        if (!fstabMap.empty()) {
            for (const auto &device : fstabMap.keys()) {
                if (urlSource.path().startsWith(device)) {
                    urlSource.setPath(urlSource.path().replace(0, device.size(), fstabMap[device]));
                    break;
                }
            }
        }

        if (!stateCheck())
            return false;

        if (FileUtils::isTrashFile(urlSource)) {
            completeFilesCount++;
            completeSourceFiles.append(urlSource);
            continue;
        }

        // url是否可以删除 canrename
        if (!isCanMoveToTrash(urlSource, &result)) {
            if (result) {
                completeFilesCount++;
                completeSourceFiles.append(urlSource);
                continue;
            }
            return false;
        }

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(urlSource);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(urlSource, targetUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        emitCurrentTaskNotify(urlSource, targetUrl);

        // if a file names test.tar.gz
        // using baseName test and complete suffix tar.gz to build ulr like test.2.tag.gz
        const QString &baseName = fileInfo->nameOf(NameInfoType::kBaseName);
        const QString &completeSuffix = fileInfo->nameOf(NameInfoType::kCompleteSuffix);

        //! the trash url builed there must same as one fileHandler.trashFile created.
        //! the trash url should be returned by fileHandler.trashFile instead of using same algorihm on two place
        //! to keep same url. todo(lyg)
        const QUrl &trashUrl = buildTrashUrl(baseName, completeSuffix);

        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.trashFile(urlSource);
        if (trashSucc) {
            completeFilesCount++;
            completeTargetFiles.append(trashUrl);
            emitProgressChangedNotify(completeFilesCount);
            completeSourceFiles.append(urlSource);
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

QUrl DoMoveToTrashFilesWorker::buildTrashUrl(const QString &baseName, const QString &completeSuffix)
{
    // gio trash no trash url return, so build trash url before trash
    // file:///aaa/bbb/ccc.txt -> trash:///ccc.txt, if trash:///ccc.txt exists, return trash:///ccc.2.txt

    QUrl url;
    url.setScheme("trash");

    int i = 1;
    while (1) {
        if (i > 1000000)   // to avoid endless loop
            break;

        QString path;
        if (i == 1)
            path = "/" + baseName ;
        else
            path = QString("/" + baseName + "." + "%1").arg(i);

        if (!completeSuffix.isEmpty())
            path = path + "." + completeSuffix;
        url.setPath(path);

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (fileInfo->exists())
            ++i;
        else
            break;
    }
    return url;
}
