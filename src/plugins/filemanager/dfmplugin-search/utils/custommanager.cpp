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
#include "custommanager.h"
#include "searchhelper.h"

#include <QDebug>

using namespace dfmplugin_search;

CustomManager::CustomManager()
{
}

CustomManager *CustomManager::instance()
{
    static CustomManager ins;
    return &ins;
}

bool dfmplugin_search::CustomManager::registerCustomInfo(const QString &scheme, const QVariantMap &properties)
{
    if (isRegisted(scheme))
        return false;

    customInfos.insert(scheme, properties);
    return true;
}

bool CustomManager::isRegisted(const QString &scheme) const
{
    return customInfos.contains(scheme);
}

bool CustomManager::isDisableSearch(const QUrl &url)
{
    QString scheme = url.scheme();
    if (SearchHelper::scheme() == scheme) {
        const QUrl &targetUrl = SearchHelper::searchTargetUrl(url);
        scheme = targetUrl.scheme();
    }

    if (!customInfos.contains(scheme))
        return false;

    const auto &property = customInfos[scheme];
    return property.value(CustomKey::kDisableSearch, false).toBool();
}

QString CustomManager::redirectedPath(const QUrl &url)
{
    QString scheme = url.scheme();
    if (SearchHelper::scheme() == scheme) {
        const QUrl &targetUrl = SearchHelper::searchTargetUrl(url);
        scheme = targetUrl.scheme();
    }

    if (!customInfos.contains(scheme))
        return "";

    const auto &property = customInfos[scheme];
    QString path = property.value(CustomKey::kRedirectedPath, "").toString();
    if (path.isEmpty())
        return "";

    auto targetPath = url.path();
    if (path.endsWith('/') && !targetPath.isEmpty())
        path = path.left(path.length() - 1);

    return (path + targetPath);
}
