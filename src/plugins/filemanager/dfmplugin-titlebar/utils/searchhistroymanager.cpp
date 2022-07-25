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

#include <QDebug>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

SearchHistroyManager *SearchHistroyManager::instance()
{
    static SearchHistroyManager instance;
    return &instance;
}

SearchHistroyManager::SearchHistroyManager(QObject *parent)
    : QObject(parent)
{
}

QStringList SearchHistroyManager::toStringList()
{
    return Application::appObtuselySetting()->value("Cache", "SearchHistroy").toStringList();
}

void SearchHistroyManager::writeIntoSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return;

    QStringList list = toStringList();

    list << keyword;

    Application::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
}

bool SearchHistroyManager::removeSearchHistory(QString keyword)
{
    if (keyword.isEmpty())
        return false;

    bool ret = false;
    QStringList list = toStringList();
    if (list.removeOne(keyword)) {
        Application::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
        ret = true;
    } else {
        qWarning() << keyword << "not exist in history";
    }
    return ret;
}

void SearchHistroyManager::clearHistory()
{
    QStringList list;

    Application::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
}
