// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagfilehelper.h"
#include "events/tageventcaller.h"

#include <QUrl>

DPTAG_USE_NAMESPACE
TagFileHelper *TagFileHelper::instance()
{
    static TagFileHelper ins;
    return &ins;
}

TagFileHelper::TagFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool TagFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        if (url.fragment().isEmpty()) {
            redirectedFileUrls.append(url);
        } else {
            QUrl redirectUrl = QUrl::fromLocalFile(url.fragment(QUrl::FullyEncoded));
            redirectedFileUrls.append(redirectUrl);
        }
    }

    TagEventCaller::sendOpenFiles(windowId, redirectedFileUrls);
    return true;
}
