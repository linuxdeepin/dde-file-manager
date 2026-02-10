// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    if (isRegisted(scheme)) {
        fmDebug() << "Scheme already registered:" << scheme;
        return false;
    }

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

    QString finalPath = path + targetPath;
    fmInfo() << "Redirected path for URL" << url.toString() << ":" << finalPath;
    return finalPath;
}

bool CustomManager::isUseNormalMenu(const QString &scheme)
{
    const auto &property = customInfos[scheme];
    if (property.contains(CustomKey::kUseNormalMenu) && property.value(CustomKey::kUseNormalMenu).toBool())
        return true;

    return false;
}
