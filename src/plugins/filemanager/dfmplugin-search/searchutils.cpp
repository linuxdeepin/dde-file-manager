/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "searchutils.h"

#include <QUrlQuery>

const char *const kScheme = "search";

bool SearchUtils::isSearchFile(const QUrl &url)
{
    return kScheme == url.scheme();
}

QString SearchUtils::scheme()
{
    return kScheme;
}

QUrl SearchUtils::searchedFileUrl(const QUrl &searchUrl)
{
    if (!isSearchFile(searchUrl))
        return {};

    return searchUrl.fragment(QUrl::FullyDecoded);
}

QUrl SearchUtils::searchTargetUrl(const QUrl &searchUrl)
{
    if (!isSearchFile(searchUrl))
        return {};

    QUrlQuery query(searchUrl.query());
    return QUrl(query.queryItemValue("url", QUrl::FullyDecoded));
}

QString SearchUtils::searchKeyword(const QUrl &searchUrl)
{
    if (!isSearchFile(searchUrl))
        return {};

    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("keyword", QUrl::FullyDecoded);
}

QUrl SearchUtils::setSearchedFileUrl(const QUrl &searchUrl, const QString &searchedFile)
{
    if (!isSearchFile(searchUrl))
        return {};

    QUrl url(searchUrl);
    url.setFragment(searchedFile, QUrl::DecodedMode);
    return url;
}

QUrl SearchUtils::setSearchKeyword(const QUrl &searchUrl, const QString &keyword)
{
    if (!isSearchFile(searchUrl))
        return {};

    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", keyword);
    url.setQuery(query);

    return url;
}

QUrl SearchUtils::setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl)
{
    if (!isSearchFile(searchUrl))
        return {};

    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("url");
    query.addQueryItem("url", targetUrl.toString());
    url.setQuery(query);

    return url;
}

QUrl SearchUtils::fromSearchFile(const QString &filePath)
{
    QUrl url;
    url.setScheme(kScheme);
    url.setPath(filePath);

    return url;
}

QUrl SearchUtils::fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QUrl &searchedFileUrl)
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
