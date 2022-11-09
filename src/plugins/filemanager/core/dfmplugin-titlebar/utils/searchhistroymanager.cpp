/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "searchhistroymanager.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <QDateTime>
#include <QDebug>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

inline constexpr char kConfigGroupName[] { "Cache" };
inline constexpr char kConfigSearchHistroy[] { "SearchHistroy" };
inline constexpr char kConfigIPHistroy[] { "IPHistroy" };

inline constexpr char kKeyIP[] { "ip" };
inline constexpr char kKeyLastAccessed[] { "lastAccessed" };

SearchHistroyManager *SearchHistroyManager::instance()
{
    static SearchHistroyManager instance;
    return &instance;
}

SearchHistroyManager::SearchHistroyManager(QObject *parent)
    : QObject(parent)
{
}

QStringList SearchHistroyManager::getSearchHistroy()
{
    return Application::appObtuselySetting()->value(kConfigGroupName, kConfigSearchHistroy).toStringList();
}

QList<IPHistroyData> SearchHistroyManager::getIPHistory()
{
    QList<IPHistroyData> data;
    const auto &list = Application::appObtuselySetting()->value(kConfigGroupName, kConfigIPHistroy).toList();
    for (const auto &item : list) {
        const auto &map = item.toMap();
        const auto &ip = map.value(kKeyIP).toString();
        const auto &time = map.value(kKeyLastAccessed).toString();
        if (ip.isEmpty() || time.isEmpty())
            continue;

        data << IPHistroyData(ip, QDateTime::fromString(time, Qt::ISODate));
    }

    return data;
}

void SearchHistroyManager::writeIntoSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return;

    QStringList list = getSearchHistroy();

    list << keyword;

    Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigSearchHistroy, list);
}

void SearchHistroyManager::writeIntoIPHistory(const QString &ipAddr)
{
    if (ipAddr.isEmpty())
        return;

    auto history = getIPHistory();
    IPHistroyData data(ipAddr, QDateTime::currentDateTime());
    if (history.contains(data)) {
        int index = history.indexOf(data);
        history.replace(index, data);
    } else {
        history << data;
    }

    QVariantList list;
    for (const auto &item : history) {
        if (item.isRecentlyAccessed())
            list << item.toVariantMap();
    }

    Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigIPHistroy, list);
}

bool SearchHistroyManager::removeSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return false;

    bool ret = false;
    QStringList list = getSearchHistroy();
    if (list.removeOne(keyword)) {
        Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigSearchHistroy, list);
        ret = true;
    } else {
        qWarning() << keyword << "not exist in history";
    }
    return ret;
}

void SearchHistroyManager::clearHistory(const QStringList &schemeFilters)
{
    if (schemeFilters.isEmpty()) {
        QStringList list;
        Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigSearchHistroy, list);
    } else {
        QStringList historyList = Application::appObtuselySetting()->value(kConfigGroupName, kConfigSearchHistroy).toStringList();
        for (const QString &data : historyList) {
            QUrl url(data);
            if (url.isValid() && schemeFilters.contains(url.scheme() + "://")) {
                historyList.removeOne(data);
            }
        }
        Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigSearchHistroy, historyList);
    }
}
