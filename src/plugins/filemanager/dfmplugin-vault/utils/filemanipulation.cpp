/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "filemanipulation.h"
#include "events/vaulteventcaller.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QFileInfo>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
using namespace dfmplugin_vault;
FileManipulation::FileManipulation(QObject *parent)
    : QObject(parent)
{
}

bool FileManipulation::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QUrl redirectedFileUrl = delegateServIns->urlTransform(url);
        redirectedFileUrls << redirectedFileUrl;
    }

    if (!redirectedFileUrls.isEmpty())
        VaultEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    return true;
}

bool FileManipulation::writeToClipBoardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        redirectedFileUrls << delegateServIns->urlTransform(url);
    }

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);

    return true;
}

JobHandlePointer FileManipulation::moveToTrashHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : sources) {
        redirectedFileUrls << delegateServIns->urlTransform(url);
    }

    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles,
                                          windowId,
                                          redirectedFileUrls, flags, nullptr);
    return {};
}

JobHandlePointer FileManipulation::deletesHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(flags)
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : sources) {
        redirectedFileUrls << delegateServIns->urlTransform(url);
    }

    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles,
                                          windowId,
                                          redirectedFileUrls, flags, nullptr);
    return {};
}

JobHandlePointer FileManipulation::copyHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (FileUtils::isComputerDesktopFile(url) || FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }
    QUrl url = delegateServIns->urlTransform(target);
    dpfSignalDispatcher->publish(GlobalEventType::kCopy, windowId, actualUrls, url, flags, nullptr);
    return {};
}

JobHandlePointer FileManipulation::cutHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> actualUrls;
    for (const QUrl &url : sources) {
        if (FileUtils::isComputerDesktopFile(url) || FileUtils::isTrashDesktopFile(url)) {
            continue;
        } else {
            actualUrls << url;
        }
    }
    QUrl url = delegateServIns->urlTransform(target);
    dpfSignalDispatcher->publish(GlobalEventType::kCutFile, windowId, actualUrls, url, flags, nullptr);
    return {};
}

bool FileManipulation::mkdirHandle(const quint64 windowId, const QUrl url, QString *errore)
{
    QUrl dirUrl = delegateServIns->urlTransform(url);
    return dpfSignalDispatcher->publish(GlobalEventType::kMkdir,
                                                 windowId,
                                                 dirUrl);
}

bool FileManipulation::touchFileHandle(const quint64 windowId, const QUrl url, QString *error, const Global::CreateFileType type)
{
    QUrl dirUrl = delegateServIns->urlTransform(url);
    return dpfSignalDispatcher->publish(GlobalEventType::kTouchFile,
                                                 windowId,
                                                 dirUrl,
                                                 type, *error);
}

bool FileManipulation::renameHandle(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags, QString *error)
{
    QUrl ourl = delegateServIns->urlTransform(oldUrl);
    QUrl nurl = delegateServIns->urlTransform(newUrl);
    return dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, windowId, ourl, nurl, flags);
}

bool FileManipulation::renameFilesHandle(const quint64 windowId, const QList<QUrl> urlList, const QPair<QString, QString> replacePair, bool flg)
{
    QList<QUrl> actualUrls;
    for (const QUrl &url : urlList) {
        actualUrls << delegateServIns->urlTransform(url);
    }
    return dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair, flg);
}

bool FileManipulation::renameFilesHandleAddText(const quint64 windowId, const QList<QUrl> urlList, const QPair<QString, AbstractJobHandler::FileNameAddFlag> replacePair)
{
    QList<QUrl> actualUrls;
    for (const QUrl &url : urlList) {
        actualUrls << delegateServIns->urlTransform(url);
    }
    return dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, windowId, actualUrls, replacePair);
}
