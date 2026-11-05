// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharefilehelper.h"
#include "shareutils.h"
#include "events/shareeventscaller.h"

#include <QUrl>

#include <dfm-base/file/local/localfilehandler.h>

DPMYSHARES_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
ShareFileHelper *ShareFileHelper::instance()
{
    static ShareFileHelper ins;
    return &ins;
}

ShareFileHelper::ShareFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool ShareFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != ShareUtils::scheme())
        return false;

    ShareEventsCaller::sendOpenDirs(windowId, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow);
    return true;
}

bool ShareFileHelper::setPermissionHandle(const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, bool *ok, QString *error)
{
    Q_UNUSED(windowId)

    if (url.scheme() != ShareUtils::scheme())
        return false;

    const QUrl &localUrl = ShareUtils::convertToLocalUrl(url);
    LocalFileHandler fileHandler;

    bool succ = fileHandler.setPermissions(localUrl, permissions);
    if (!succ && error) {
        *error = fileHandler.errorString();
        fmWarning() << "Failed to set permissions for file:" << localUrl << "error:" << *error;
    }

    if (ok)
        *ok = succ;

    return true;
}
