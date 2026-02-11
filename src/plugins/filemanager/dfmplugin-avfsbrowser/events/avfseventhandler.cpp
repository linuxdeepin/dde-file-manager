// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfseventhandler.h"
#include "utils/avfsutils.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-io/dfileinfo.h>

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

        bool isDir { DFMIO::DFileInfo(url).attribute(DFMIO::DFileInfo::AttributeID::kStandardIsDir).toBool() };
        if (AvfsUtils::isSupportedArchives(url.path())) {
            takeHandle = true;
            archives << AvfsUtils::localArchiveToAvfsUrl(url);
        } else if (url.path().startsWith(AvfsUtils::avfsMountPoint()) && isDir) {
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
            fmWarning() << "open files failed: " << others;
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

    if (!AvfsUtils::archivePreviewEnabled())
        return false;

    if (url.scheme() == AvfsUtils::scheme() || url.path().startsWith(AvfsUtils::avfsMountPoint() + "/")) {
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
