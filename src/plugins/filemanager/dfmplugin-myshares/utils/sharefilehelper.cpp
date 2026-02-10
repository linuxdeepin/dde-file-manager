// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharefilehelper.h"
#include "shareutils.h"
#include "events/shareeventscaller.h"

#include <QUrl>

DPMYSHARES_USE_NAMESPACE
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
