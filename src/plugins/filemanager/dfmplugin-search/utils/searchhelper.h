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
#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H

#include "dfmplugin_search_global.h"

#include <QUrl>

DPSEARCH_BEGIN_NAMESPACE

class SearchHelper final
{
public:
    inline static QString scheme()
    {
        return "search";
    }

    static QUrl rootUrl();
    static bool isRootUrl(const QUrl &url);
    static bool isSearchFile(const QUrl &url);
    static QUrl searchedFileUrl(const QUrl &searchUrl);
    static QUrl searchTargetUrl(const QUrl &searchUrl);
    static QString searchKeyword(const QUrl &searchUrl);
    static QString searchTaskId(const QUrl &searchUrl);

    static QUrl setSearchedFileUrl(const QUrl &searchUrl, const QString &searchedFile);
    static QUrl setSearchKeyword(const QUrl &searchUrl, const QString &keyword);
    static QUrl setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl);
    static QUrl setSearchTaskId(const QUrl &searchUrl, const QString &taskId);

    static QUrl fromSearchFile(const QString &filePath);
    static QUrl fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QString &taskId, const QUrl &searchedFileUrl);

private:
    SearchHelper() = delete;
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHHELPER_H
