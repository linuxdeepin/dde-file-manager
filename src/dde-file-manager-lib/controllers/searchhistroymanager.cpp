/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

void SearchHistroyManager::clearHistory()
{
    QStringList list;

    DFMApplication::appObtuselySetting()->setValue("Cache", "SearchHistroy", list);
}

