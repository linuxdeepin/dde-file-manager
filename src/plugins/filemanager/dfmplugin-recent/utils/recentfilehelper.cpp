// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilehelper.h"
#include "recentmanager.h"
#include "events/recenteventcaller.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/event/event.h>

#include <DDialog>
#include <DDesktopServices>
#include <dtkgui_global.h>
#include <dtkwidget_global.h>

#include <dfm-io/dfmio_utils.h>

#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace dfmplugin_recent;

RecentFileHelper *RecentFileHelper::instance()
{
    static RecentFileHelper instance;
    return &instance;
}

bool RecentFileHelper::setPermissionHandle(const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, bool *ok, QString *error)
{
    if (Global::Scheme::kRecent != url.scheme()) {
        fmDebug() << "URL scheme is not recent, ignoring permission request";
        return false;
    }

    Q_UNUSED(windowId)

    const QUrl &localUrl = RecentHelper::urlTransform(url);
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;

    bool succ = fileHandler.setPermissions(localUrl, permissions);
    if (!succ && error) {
        *error = fileHandler.errorString();
        fmWarning() << "Failed to set permissions for file:" << localUrl << "error:" << *error;
    }

    if (ok)
        *ok = succ;

    return true;
}

bool RecentFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId)
    Q_UNUSED(sources)
    Q_UNUSED(flags)

    return target.scheme() == RecentHelper::scheme();
}

bool RecentFileHelper::copyFile(const quint64, const QList<QUrl>, const QUrl target, const AbstractJobHandler::JobFlags)
{
    return target.scheme() == RecentHelper::scheme();
}

bool RecentFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty()) {
        fmWarning() << "Move to trash failed - empty sources list";
        return false;
    }

    if (sources.first().scheme() != RecentHelper::scheme()) {
        fmDebug() << "Move to trash ignored - first source is not recent scheme:" << sources.first().scheme();
        return false;
    }

    Q_UNUSED(windowId)
    Q_UNUSED(flags)

    RecentHelper::removeRecent(sources);

    return true;
}

bool RecentFileHelper::openFileInPlugin(quint64 winId, QList<QUrl> urls)
{
    if (urls.isEmpty()) {
        fmWarning() << "Open file in plugin failed - empty URLs list";
        return false;
    }

    if (urls.first().scheme() != RecentHelper::scheme()) {
        fmDebug() << "Open file in plugin ignored - first URL is not recent scheme:" << urls.first().scheme();
        return false;
    }

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::Scheme::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendOpenFiles(winId, redirectedFileUrls);
    return true;
}

bool RecentFileHelper::writeUrlsToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty()) {
        fmWarning() << "Write to clipboard failed - empty URLs list";
        return false;
    }

    if (urls.first().scheme() != RecentHelper::scheme()) {
        fmDebug() << "Write to clipboard ignored - first URL is not recent scheme:" << urls.first().scheme();
        return false;
    }

    if (action == ClipBoard::ClipboardAction::kCutAction)
        return true;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::Scheme::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendWriteToClipboard(windowId, action, redirectedFileUrls);
    return true;
}

bool RecentFileHelper::openFileInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty()) {
        fmWarning() << "Open in terminal failed - empty URLs list";
        return false;
    }

    if (urls.first().scheme() != RecentHelper::scheme()) {
        fmDebug() << "Open in terminal ignored - first URL is not recent scheme:" << urls.first().scheme();
        return false;
    }

    Q_UNUSED(windowId);

    return true;
}

RecentFileHelper::RecentFileHelper(QObject *parent)
    : QObject(parent)
{
}
