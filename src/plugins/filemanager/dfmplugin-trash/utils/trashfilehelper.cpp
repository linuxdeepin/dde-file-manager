/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/dialogmanager.h"

#include <dfm-framework/event/event.h>
#include <dfm-io/dfmio_utils.h>

#include <QUrl>

DPTRASH_USE_NAMESPACE
TrashFileHelper *TrashFileHelper::instance()
{
    static TrashFileHelper ins;
    return &ins;
}

TrashFileHelper::TrashFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool TrashFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    if (sources.isEmpty())
        return true;

    const QUrl &urlSource = sources.first();
    if (Q_UNLIKELY(DFMBASE_NAMESPACE::FileUtils::isGvfsFile(urlSource) || DFMIO::DFMUtils::fileIsRemovable(urlSource))) {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                     windowId,
                                     sources, flags, nullptr);
    } else {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash,
                                     windowId,
                                     sources, flags, nullptr);
    }
    return true;
}

bool TrashFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash,
                                 windowId,
                                 sources, flags, nullptr);
    return true;
}

bool TrashFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : sources) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCleanTrash,
                                 windowId,
                                 redirectedFileUrls,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return true;
}

bool TrashFileHelper::deleteFile(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : sources) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCleanTrash,
                                 windowId,
                                 redirectedFileUrls,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return true;
}

bool TrashFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

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

    if (isOpenFile) {
        QString strMsg = QObject::tr("Unable to open items in the trash, please restore it first");
        DialogManagerInstance->showMessageDialog(DFMBASE_NAMESPACE::DialogManager::kMsgWarn, strMsg);
    }
    return true;
}

bool TrashFileHelper::writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        redirectedFileUrls << TrashHelper::toLocalFile(url);
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);

    return true;
}
