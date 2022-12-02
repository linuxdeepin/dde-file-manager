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
#include "vaultfilehelper.h"
#include "vaulthelper.h"

#include "events/vaulteventcaller.h"

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/event/event.h>

#include <QUrl>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPVAULT_USE_NAMESPACE
VaultFileHelper *VaultFileHelper::instance()
{
    static VaultFileHelper ins;
    return &ins;
}

VaultFileHelper::VaultFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool VaultFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCutFile, windowId, actualUrls, url, flags, nullptr);

    return true;
}

bool VaultFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url) || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }

    // if use &, transUrlsToLocal return value will free, and url is invalid, app crash, the same below
    const QUrl url = transUrlsToLocal({ target }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopy, windowId, actualUrls, url, flags, nullptr);
    return true;
}

bool VaultFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls = transUrlsToLocal(sources);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 redirectedFileUrls, flags, nullptr);
    return true;
}

bool VaultFileHelper::deleteFile(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(sources);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 redirectedFileUrls, flags, nullptr);
    return true;
}

bool VaultFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(urls);

    if (!redirectedFileUrls.isEmpty())
        VaultEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    return true;
}

bool VaultFileHelper::renameFile(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (oldUrl.scheme() != scheme())
        return false;

    const QUrl ourl = transUrlsToLocal({ oldUrl }).first();
    const QUrl nurl = transUrlsToLocal({ newUrl }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFile, windowId, ourl, nurl, flags);

    return true;
}

bool VaultFileHelper::makeDir(const quint64 windowId, const QUrl url)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdir, windowId, dirUrl);

    return true;
}

bool VaultFileHelper::touchFile(const quint64 windowId, const QUrl url, const DFMGLOBAL_NAMESPACE::CreateFileType type, QString *error)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFile,
                                 windowId, dirUrl, type, *error);
    return true;
}

bool VaultFileHelper::touchCustomFile(const quint64 windowId, const QUrl url, const QUrl tempUrl, QString *error)
{
    if (url.scheme() != scheme())
        return false;

    const QUrl dirUrl = transUrlsToLocal({ url }).first();
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFile,
                                 windowId, dirUrl, tempUrl, *error);
    return true;
}

bool VaultFileHelper::writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);

    return true;
}

bool VaultFileHelper::renameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> replacePair, bool flg)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> actualUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair, flg);

    return true;
}

bool VaultFileHelper::renameFilesAddText(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> replacePair)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> actualUrls = transUrlsToLocal(urls);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair);

    return true;
}

bool VaultFileHelper::checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    Q_UNUSED(urls);

    if (urlTo.scheme() != scheme())
        return false;

    if (*action == Qt::MoveAction) {
        *action = Qt::CopyAction;
        return true;
    }

    return false;
}

bool VaultFileHelper::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (toUrl.scheme() == scheme()) {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopy,
                                     0,
                                     fromUrls,
                                     toUrl,
                                     DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    }

    return false;
}

QList<QUrl> VaultFileHelper::transUrlsToLocal(const QList<QUrl> &urls)
{
    QList<QUrl> urlsTrans {};
    if (VaultHelper::instance()->urlsToLocal(urls, &urlsTrans))
        return urlsTrans;

    return urls;
}
