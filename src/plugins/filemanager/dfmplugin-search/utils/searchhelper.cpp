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

static inline QString parseDecodedComponent(const QString &data)
{
    return QString(data).replace(QLatin1Char('%'), QStringLiteral("%25"));
}

QUrl SearchHelper::rootUrl()
{
    return fromSearchFile("/");
}

bool SearchHelper::isRootUrl(const QUrl &url)
{
    return url.path() == rootUrl().path();
}

bool SearchHelper::isSearchFile(const QUrl &url)
{
    return url.scheme() == scheme();
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

QString SearchHelper::searchTaskId(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("taskId", QUrl::FullyDecoded);
}

QUrl SearchHelper::setSearchKeyword(const QUrl &searchUrl, const QString &keyword)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("url");
    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTaskId(const QUrl &searchUrl, const QString &taskId)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("taskId");
    query.addQueryItem("taskId", taskId);
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

QUrl SearchHelper::fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QString &taskId)
{
    QUrl url = rootUrl();
    QUrlQuery query;

    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    query.addQueryItem("taskId", taskId);
    url.setQuery(query);

    return url;
}

DPSEARCH_END_NAMESPACE
