/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "avfseventhandler.h"
#include "utils/avfsutils.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/clipboard.h"

#include <dfm-framework/event/event.h>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsEventHandler *AvfsEventHandler::instance()
{
    static AvfsEventHandler ins;
    return &ins;
}

bool AvfsEventHandler::hookOpenFiles(quint64 winId, const QList<QUrl> &urls)
{
    if (!AvfsUtils::archivePreviewEnabled())
        return false;

    bool takeHandle = false;
    QList<QUrl> archives, others;
    for (auto url : urls) {
        if (url.scheme() != Global::Scheme::kFile && url.scheme() != AvfsUtils::scheme())
            return false;
        if (url.scheme() == AvfsUtils::scheme()) {
            takeHandle = true;
            url = AvfsUtils::avfsUrlToLocal(url);
        }

        if (AvfsUtils::isSupportedArchives(url.path())) {
            takeHandle = true;
            archives << AvfsUtils::localArchiveToAvfsUrl(url);
        } else if (url.path().startsWith(AvfsUtils::avfsMountPoint()) && DecoratorFileInfo(url.path()).isDir()) {
            takeHandle = true;
            archives << AvfsUtils::localUrlToAvfsUrl(url);
        } else {
            others << url;
        }
    }

    if (!takeHandle)
        return false;

    if (archives.count() > 0)
        openArchivesAsDir(winId, archives);

    if (others.count() > 0) {
        LocalFileHandler handler;
        bool ok = std::all_of(others.cbegin(), others.cend(), [&](const QUrl &u) { return handler.openFile({ u }); });
        if (!ok)
            qWarning() << "open files failed: " << others;
    }
    return true;
}

bool AvfsEventHandler::hookEnterPressed(quint64 winId, const QList<QUrl> &urls)
{
    return hookOpenFiles(winId, urls);
}

bool AvfsEventHandler::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == AvfsUtils::scheme()) {
        *mapGroup = AvfsUtils::seperateUrl(url);
        return true;
    }

    return false;
}

void AvfsEventHandler::openArchivesAsDir(quint64 winId, const QList<QUrl> &urls)
{
    if (urls.count() == 1) {
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, urls.first());
    } else {
        std::for_each(urls.cbegin(), urls.cend(), [](const QUrl &url) {
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
        });
    }
}

void AvfsEventHandler::writeToClipbord(quint64 winId, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, winId, ClipBoard::kCopyAction, urls);
}

void AvfsEventHandler::showProperty(const QList<QUrl> &urls)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}
