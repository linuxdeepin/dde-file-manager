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
#include "opticalfilehelper.h"
#include "opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QDir>

DPOPTICAL_USE_NAMESPACE
OpticalFileHelper *OpticalFileHelper::instance()
{
    static OpticalFileHelper ins;
    return &ins;
}

OpticalFileHelper::OpticalFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool OpticalFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    Q_UNUSED(windowId)
    Q_UNUSED(flags)
    pasteFilesHandle(sources, target, false);

    return true;
}

bool OpticalFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;

    if (target.scheme() != scheme())
        return false;

    Q_UNUSED(windowId)
    Q_UNUSED(flags)
    pasteFilesHandle(sources, target);
    return true;
}

bool OpticalFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : sources) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            continue;
        if (!OpticalHelper::burnIsOnDisc(url))
            redirectedFileUrls.push_back(QUrl::fromLocalFile(backer));
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles, windowId, redirectedFileUrls, flags, nullptr);
    return true;
}

bool OpticalFileHelper::openFileInPlugin(quint64 winId, QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        redirectedFileUrls << QUrl::fromLocalFile(MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString());
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, winId, redirectedFileUrls);
    return true;
}

bool OpticalFileHelper::linkFile(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence)
{
    if (url.scheme() != scheme())
        return false;

    QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
    if (backer.isEmpty())
        return false;

    QUrl redirectedFileUrl { QUrl::fromLocalFile(backer) };
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlink, windowId, redirectedFileUrl, link, force, silence);
    return true;
}

bool OpticalFileHelper::writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    if (action != DFMBASE_NAMESPACE::ClipBoard::ClipboardAction::kCopyAction)
        return false;
    // only write file on disc
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        MasteredMediaFileInfo info(url);
        QUrl backerUrl { QUrl::fromLocalFile(info.extraProperties()["mm_backer"].toString()) };
        if (!OpticalHelper::localStagingRoot().isParentOf(backerUrl))
            redirectedFileUrls.push_back(backerUrl);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);
    return true;
}

bool OpticalFileHelper::openFileInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    const QString &currentDir = QDir::currentPath();

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            return false;
        redirectedFileUrls << QUrl::fromLocalFile(backer);
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminal, windowId, redirectedFileUrls);
    QDir::setCurrent(currentDir);
    return true;
}

void OpticalFileHelper::pasteFilesHandle(const QList<QUrl> sources, const QUrl target, bool isCopy)
{
    if (!OpticalHelper::isBurnEnabled()) {
        qInfo() << "Burn is disabled, cannot paste files to disc: " << sources;
        return;
    }
    dpfSlotChannel->push("dfmplugin_burn", "slot_PasteTo", sources, target, isCopy);
}
