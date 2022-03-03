/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchfileoperations.h"
#include "searchhelper.h"

#include "dfm-framework/framework.h"

#include "dfm-base/dfm_event_defines.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

bool SearchFileOperations::openFilesHandle(quint64 winId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> realUrls;
    for (const QUrl &url : urls) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    return dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles, winId, realUrls);
}

bool SearchFileOperations::renameFileHandle(const quint64 winId, const QUrl oldUrl, const QUrl newUrl, QString *error)
{
    Q_UNUSED(error)

    return dpfInstance.eventDispatcher().publish(GlobalEventType::kRenameFile, winId, SearchHelper::searchedFileUrl(oldUrl), newUrl);
}

bool SearchFileOperations::openInTerminalHandle(const quint64 winId, const QList<QUrl> urls, QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> realUrls;
    for (const QUrl &url : urls) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    return dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenInTerminal, winId, realUrls);
}

bool SearchFileOperations::writeToClipBoardHandle(const quint64 winId,
                                                  const ClipBoard::ClipboardAction action,
                                                  const QList<QUrl> urls)
{
    QList<QUrl> realUrls;
    for (const QUrl &url : urls) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    return dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard, winId, action, realUrls);
}

JobHandlePointer SearchFileOperations::moveToTrashHandle(const quint64 winId,
                                                         const QList<QUrl> sources,
                                                         const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> realUrls;
    for (const QUrl &url : sources) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    dpfInstance.eventDispatcher().publish(GlobalEventType::kMoveToTrash, winId, realUrls, flags);
    return {};
}

JobHandlePointer SearchFileOperations::deleteFilesHandle(const quint64 winId,
                                                         const QList<QUrl> sources,
                                                         const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> realUrls;
    for (const QUrl &url : sources) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles, winId, realUrls, flags);
    return {};
}

DPSEARCH_END_NAMESPACE
