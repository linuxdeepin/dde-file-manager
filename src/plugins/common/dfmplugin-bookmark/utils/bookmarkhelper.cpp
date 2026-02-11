// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkhelper.h"
#include "controller/defaultitemmanager.h"

#include <QUrl>
#include <QIcon>
#include <QVariantMap>
#include <QDebug>

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

bool BookMarkHelper::isValidQuickAccessConf(const QVariantList &list)
{
    if (list.isEmpty())
        return false;
    const QStringList &preDefNames {
        DefaultItemManager::instance()->preDefItemUrls().keys()
    };
    const QStringList &defaultNames {
        DefaultItemManager::instance()->defaultItemUrls().keys()
    };
    int totalItemCount { preDefNames.count() + defaultNames.count() };
    int curItemCount { 0 };

    for (const QVariant &data : list) {
        const auto &bookMarkMap { data.toMap() };
        const auto &url { bookMarkMap.value("url").toUrl() };
        const auto &name { bookMarkMap.value("name").toString() };
        if (!url.isValid() || name.isEmpty()) {
            fmWarning() << "Invalid quick access data:" << bookMarkMap;
            return false;
        }

        // 检查默认和预定义的 Item 一定在列表中
        if (preDefNames.contains(name) || defaultNames.contains(name))
            ++curItemCount;
    }

    return totalItemCount == curItemCount;
}

BookMarkHelper::BookMarkHelper(QObject *parent)
    : QObject(parent) { }
