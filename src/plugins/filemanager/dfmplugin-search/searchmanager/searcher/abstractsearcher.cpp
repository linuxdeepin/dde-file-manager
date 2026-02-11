// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractsearcher.h"

DPSEARCH_USE_NAMESPACE

AbstractSearcher::AbstractSearcher(const QUrl &url, const QString &key, QObject *parent)
    : QObject(parent),
      searchUrl(url),
      keyword(key)
{
}

QList<QUrl> AbstractSearcher::takeAllUrls()
{
    // 将 DFMSearchResultMap 转换为 QList<QUrl>，用于向后兼容
    return takeAll().keys();
}
