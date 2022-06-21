/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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

#include "trashfilehelper.h"
#include "trashhelper.h"
#include "events/trasheventcaller.h"

#include "dfm-framework/framework.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-io/dfmio_utils.h>

#include <QFileInfo>

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

bool TrashFileHelper::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    bool isOpenFile = false;
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QUrl redirectedFileUrl = TrashHelper::toLocalFile(url);
        QFileInfo fileInfo(redirectedFileUrl.path());
        if (fileInfo.isFile()) {
            isOpenFile = true;
            continue;
        }
        redirectedFileUrls << redirectedFileUrl;
    }
    if (!redirectedFileUrls.isEmpty())
        TrashEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    // Todo(yanghao)
    if (isOpenFile) {
        QString strMsg = QObject::tr("Unable to open items in the trash, please restore it first");
        DialogManagerInstance->showMessageDialog(DialogManager::kMsgWarn, strMsg);
    }
    return true;
}

bool TrashFileHelper::writeToClipBoardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);

    return true;
}

JobHandlePointer TrashFileHelper::moveToTrashHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    // Todo(yanghao&lxs):回收站彻底删除
    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : sources) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }
    dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash,
                                          windowId,
                                          redirectedFileUrls,
                                          AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return {};
}

JobHandlePointer TrashFileHelper::moveFromTrashHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl &targetUrl, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : sources) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }
    dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                          windowId,
                                          redirectedFileUrls,
                                          targetUrl,
                                          AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return {};
}

JobHandlePointer TrashFileHelper::deletesHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    // Todo(yanghao&lxs):回收站彻底删除
    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : sources) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }
    dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash,
                                          windowId,
                                          redirectedFileUrls,
                                          AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return {};
}

JobHandlePointer TrashFileHelper::copyHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash,
                                          windowId,
                                          sources, nullptr);
    return {};
}

JobHandlePointer TrashFileHelper::cutHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return nullptr;

    const QUrl &urlSource = sources.first();
    if (Q_UNLIKELY(FileUtils::isGvfsFile(urlSource) || DFMIO::DFMUtils::fileIsRemovable(urlSource))) {
        dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles,
                                              windowId,
                                              sources, flags, nullptr);
    } else {
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash,
                                              windowId,
                                              sources, flags, nullptr);
    }
    return {};
}

JobHandlePointer TrashFileHelper::restoreFromTrashHandle(const quint64 windowId, const QList<QUrl> urls, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> urlsLocal;
    for (const auto &url : urls) {
        if (url.scheme() == TrashHelper::scheme())
            urlsLocal.append(TrashHelper::toLocalFile(url));
    }

    dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash,
                                          windowId,
                                          urlsLocal,
                                          flags, nullptr);
    return {};
}
