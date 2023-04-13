// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhistroymanager.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

DFM_USE_NAMESPACE

SearchHistroyManager::SearchHistroyManager()
{

}

SearchHistroyManager::~SearchHistroyManager()
{

}

QStringList SearchHistroyManager::toStringList()
{
    return DFMApplication::appObtuselySetting()->value("Cache", "SearchHistroy").toStringList();
}

void SearchHistroyManager::writeIntoSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return;

    QStringList list = toStringList();

    list << keyword;

    DFMApplication::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
}

bool SearchHistroyManager::removeSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return false;

    bool ret = false;
    QStringList list = toStringList();
    if (list.removeOne(keyword)) {
        DFMApplication::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
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
        DFMApplication::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
    } else {
        QStringList historyList = DFMApplication::appObtuselySetting()->value("Cache", "SearchHistroy").toStringList();
        for (const QString &data : historyList) {
            QUrl url(data);
            if (url.isValid() && schemeFilters.contains(url.scheme() + "://"))
                historyList.removeAll(data);
        }
        DFMApplication::appObtuselySetting()->setValue("Cache", "SearchHistroy", historyList);
    }
}

