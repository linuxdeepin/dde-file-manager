/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#include "opticalfileshelper.h"
#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"
#include "events/opticaleventcaller.h"

#include "services/common/burn/burn_defines.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QDir>

DPOPTICAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

bool OpticalFilesHelper::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        redirectedFileUrls << QUrl::fromLocalFile(MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString());
    }
    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles, windowId, redirectedFileUrls);
    return true;
}

void OpticalFilesHelper::pasteFilesHandle(const QList<QUrl> sources, const QUrl target, bool isCopy)
{
    DSC_USE_NAMESPACE
    dpfInstance.eventDispatcher().publish(Burn::EventType::kPasteTo, sources, target, isCopy);
}

bool OpticalFilesHelper::writeUrlToClipboardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (action != ClipBoard::ClipboardAction::kCopyAction)
        return false;
    // only write file on disc
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        MasteredMediaFileInfo info(url);
        QUrl backerUrl { QUrl::fromLocalFile(info.extraProperties()["mm_backer"].toString()) };
        if (!OpticalHelper::localStagingRoot().isParentOf(backerUrl))
            redirectedFileUrls.push_back(backerUrl);
    }
    dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard, windowId, action, redirectedFileUrls);
    return !redirectedFileUrls.isEmpty();
}

bool OpticalFilesHelper::openInTerminalHandle(const quint64 windowId, const QList<QUrl> urls, QString *error)
{
    Q_UNUSED(error)

    const QString &currentDir = QDir::currentPath();

    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            return false;
        redirectedFileUrls << QUrl::fromLocalFile(backer);
    }

    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenInTerminal, windowId, redirectedFileUrls);
    QDir::setCurrent(currentDir);
    return true;
}

JobHandlePointer OpticalFilesHelper::deleteFilesHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : sources) {
        QString backer { MasteredMediaFileInfo(url).extraProperties()["mm_backer"].toString() };
        if (backer.isEmpty())
            continue;
        if (!OpticalHelper::burnIsOnDisc(url))
            redirectedFileUrls.push_back(backer);
    }

    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles, windowId, redirectedFileUrls, flags);
    return {};
}
