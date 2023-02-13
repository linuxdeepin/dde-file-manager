// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
