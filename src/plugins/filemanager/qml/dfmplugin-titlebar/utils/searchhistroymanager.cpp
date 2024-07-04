// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhistroymanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

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

    QUrl url(keyword);
    if (keyword.startsWith(url.scheme())) {
        if (keyword.startsWith(Global::Scheme::kSmb) || keyword.startsWith(Global::Scheme::kFtp) || keyword.startsWith(Global::Scheme::kSFtp)) {
            if (!url.isValid()) {
                fmDebug() << "Url is invalid, do not write it to history.";
                return;
            }
        }
    }

    QStringList list = getSearchHistroy();
    if (list.contains(keyword))
        list.removeAll(keyword);

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
    ret = list.removeOne(keyword);
    if (!ret) {
        QString keywordNoSlash = keyword;
        if (keywordNoSlash.endsWith("/")) {
            keywordNoSlash.chop(1);
            ret = list.removeOne(keywordNoSlash);
        }
    }
    if (ret)
        Application::appObtuselySetting()->setValue(kConfigGroupName, kConfigSearchHistroy, list);
    else
        fmWarning() << keyword << "not exist in history";

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
