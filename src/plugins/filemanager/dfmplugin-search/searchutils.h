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
#ifndef SEARCHUTILS_H
#define SEARCHUTILS_H

#include <QUrl>

class SearchUtils
{
public:
    explicit SearchUtils() = delete;
    static bool isSearchFile(const QUrl &url);
    static QString scheme();
    static QUrl searchedFileUrl(const QUrl &searchUrl);
    static QUrl searchTargetUrl(const QUrl &searchUrl);
    static QString searchKeyword(const QUrl &searchUrl);

    static QUrl setSearchedFileUrl(const QUrl &searchUrl, const QString &searchedFile);
    static QUrl setSearchKeyword(const QUrl &searchUrl, const QString &keyword);
    static QUrl setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl);

    static QUrl fromSearchFile(const QString &filePath);
    static QUrl fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QUrl &searchedFileUrl);
};

#endif   // SEARCHUTILS_H
