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
#include "docopyfromtrashfilesworker.h"
#include "fileoperations/copyfiles/storageinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_utils.h>
#include <dfm-io/core/diofactory.h>

#include <QUrl>
#include <QDebug>
#include <QMutex>
#include <QSettings>
#include <QStorageInfo>

DPFILEOPERATIONS_USE_NAMESPACE
DoCopyFromTrashFilesWorker::DoCopyFromTrashFilesWorker(QObject *parent)
    : FileOperateBaseWorker(parent)
{
    jobType = AbstractJobHandler::JobType::kCopyType;
}

DoCopyFromTrashFilesWorker::~DoCopyFromTrashFilesWorker()
{
    stop();
}

bool DoCopyFromTrashFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;

    doOperate();

    // 完成
    endWork();

    return true;
}

bool DoCopyFromTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sourceUrls.size();
    if (sourceUrls.count() == 0) {
        qWarning() << "sources files list is empty!";
        return false;
    }

    return true;
}

bool DoCopyFromTrashFilesWorker::initArgs()
{
    completeTargetFiles.clear();
    return AbstractWorker::initArgs();
}
/*!
 * \brief DoCopyFromTrashFilesWorker::doRestoreTrashFiles Performing a recycle bin restore
 * \return Is the recycle bin restore successful
 */
bool DoCopyFromTrashFilesWorker::doOperate()
{
    bool result = false;

    for (const auto &url : sourceUrls) {
        if (!stateCheck())
            return false;

        const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        const QUrl &targetFileUrl = DFMIO::DFMUtils::buildFilePath(this->targetUrl.toString().toStdString().c_str(),
                                                                   fileInfo->displayOf(DisPlayInfoType::kFileDisplayName).toStdString().c_str(), nullptr);

        const AbstractFileInfoPointer &targetFileInfo = InfoFactory::create<AbstractFileInfo>(targetFileUrl);
        if (!targetFileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, targetFileUrl, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                completeFilesCount++;
                continue;
            }
        }

        AbstractFileInfoPointer targetInfo = nullptr;
        if (!createParentDir(fileInfo, targetFileInfo, targetInfo, &result)) {
            if (result) {
                completeFilesCount++;
                continue;
            } else {
                return false;
            }
        }

        // read trash info
        emitCurrentTaskNotify(url, targetFileUrl);

        AbstractFileInfoPointer newTargetInfo(nullptr);
        bool ok = false;
        if (!doCheckFile(fileInfo, targetInfo, fileInfo->nameOf(NameInfoType::kFileCopyName), newTargetInfo, &ok))
            continue;

        bool copySucc = this->copyFile(url, newTargetInfo->urlOf(UrlInfoType::kUrl), DFMIO::DFile::CopyFlag::kOverwrite);
        if (copySucc) {
            completeFilesCount++;
            if (!completeSourceFiles.contains(url)) {
                completeSourceFiles.append(url);
            }
            if (!completeTargetFiles.contains(targetFileInfo->urlOf(UrlInfoType::kUrl)))
                completeTargetFiles.append(targetFileInfo->urlOf(UrlInfoType::kUrl));
            continue;
        }
        return false;
    }

    return true;
}

bool DoCopyFromTrashFilesWorker::createParentDir(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo,
                                                 AbstractFileInfoPointer &targetFileInfo, bool *result)
{
    const QUrl &fromUrl = trashInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &toUrl = restoreInfo->urlOf(UrlInfoType::kUrl);
    const QUrl &parentUrl = UrlRoute::urlParent(toUrl);
    if (!parentUrl.isValid())
        return false;
    targetFileInfo.reset();
    targetFileInfo = InfoFactory::create<AbstractFileInfo>(parentUrl);
    if (!targetFileInfo)
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    if (!DecoratorFile(parentUrl).exists()) {
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

bool DoCopyFromTrashFilesWorker::copyFile(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag)
{
    auto fileinfo = InfoFactory::create<AbstractFileInfo>(urlSource);
    if (fileinfo->isAttributes(OptInfoType::kIsDir)) {
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        if (!fileHandler.mkdir(urlTarget))
            return false;

        QString error;
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(urlSource, &error);
        if (!iterator)
            return false;
        while (iterator->hasNext()) {
            const QUrl &url = iterator->next();
            auto fileinfoNext = InfoFactory::create<AbstractFileInfo>(url);
            if (fileinfoNext->isAttributes(OptInfoType::kIsDir)) {
                return copyFile(url, DFMIO::DFMUtils::buildFilePath(urlTarget.toString().toLocal8Bit().data(), fileinfoNext->nameOf(NameInfoType::kFileCopyName).toLocal8Bit().data(), nullptr), flag);
            } else {
                DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
                bool trashSucc = fileHandler.copyFile(url,
                                                      DFMIO::DFMUtils::buildFilePath(urlTarget.toString().toLocal8Bit().data(), fileinfoNext->nameOf(NameInfoType::kFileCopyName).toLocal8Bit().data(), nullptr),
                                                      flag);
                if (!trashSucc)
                    return false;
            }
        }

    } else {
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool trashSucc = fileHandler.copyFile(urlSource, urlTarget, flag);
        return trashSucc;
    }
}
