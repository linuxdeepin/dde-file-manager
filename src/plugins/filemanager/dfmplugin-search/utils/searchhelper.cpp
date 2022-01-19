/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchhelper.h"

#include <QUrlQuery>

DPSEARCH_BEGIN_NAMESPACE

QUrl SearchHelper::rootUrl()
{
    return fromSearchFile("");
}

bool SearchHelper::isSearchFile(const QUrl &url)
{
    return url.scheme() == scheme();
}

QUrl SearchHelper::searchedFileUrl(const QUrl &searchUrl)
{
    return searchUrl.path(QUrl::FullyDecoded);
}

QUrl SearchHelper::searchTargetUrl(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return QUrl(query.queryItemValue("url", QUrl::FullyDecoded));
}

QString SearchHelper::searchKeyword(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("keyword", QUrl::FullyDecoded);
}

QUrl SearchHelper::setSearchedFileUrl(const QUrl &searchUrl, const QString &searchedFile)
{
    QUrl url(searchUrl);
    url.setPath(searchedFile, QUrl::DecodedMode);
    return url;
}

QUrl SearchHelper::setSearchKeyword(const QUrl &searchUrl, const QString &keyword)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", keyword);
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("url");
    query.addQueryItem("url", targetUrl.toString());
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QString &filePath)
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(filePath);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QUrl &searchedFileUrl)
{
    QUrl url = fromSearchFile(QString());
    QUrlQuery query;

    query.addQueryItem("url", targetUrl.toString());
    query.addQueryItem("keyword", keyword);
    url.setQuery(query);

    if (searchedFileUrl.isValid())
        url.setFragment(searchedFileUrl.toString(), QUrl::DecodedMode);

    return url;
}

DPSEARCH_END_NAMESPACE
