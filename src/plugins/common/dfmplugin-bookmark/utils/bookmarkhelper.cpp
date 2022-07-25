/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "bookmarkhelper.h"

#include <QUrl>
#include <QIcon>

#include <mutex>

using namespace dfmplugin_bookmark;

BookMarkHelper *BookMarkHelper::instance()
{
    static BookMarkHelper instance;
    return &instance;
}

QString BookMarkHelper::scheme()
{
    return "bookmark";
}

QUrl BookMarkHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

QIcon BookMarkHelper::icon()
{
    return QIcon::fromTheme("folder-bookmark-symbolic");
}

BookMarkHelper::BookMarkHelper(QObject *parent)
    : QObject(parent) {}
