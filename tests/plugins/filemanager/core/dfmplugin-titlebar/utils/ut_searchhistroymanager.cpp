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
#include "utils/searchhistroymanager.h"

#include "dfm-base/base/application/settings.h"

#include "stubext.h"

#include <gtest/gtest.h>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(SearchHistroyManagerTest, ut_getIPHistory)
{
    stub_ext::StubExt st;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<Value>(&Settings::value);
    st.set_lamda(valueFunc, [] {
        QVariantList list;
        QVariantMap map1, map2;
        map1.insert("ip", "smb://1.0.0.0");
        map1.insert("lastAccessed", "2022-08-12T09:49:24");
        map2.insert("ip", "");
        map2.insert("lastAccessed", "2022-08-12T09:49:24");

        list << map1 << map2;
        return QVariant::fromValue(list);
    });
    st.set_lamda(&Settings::setWatchChanges, [] {});

    auto data = SearchHistroyManager::instance()->getIPHistory();
    EXPECT_FALSE(data.isEmpty());
}

TEST(SearchHistroyManagerTest, ut_writeIntoIPHistory)
{
    stub_ext::StubExt st;
    st.set_lamda(&Settings::setWatchChanges, [] {});

    EXPECT_NO_FATAL_FAILURE(SearchHistroyManager::instance()->writeIntoIPHistory(""));

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    auto setValueFunc = static_cast<SetValue>(&Settings::setValue);
    st.set_lamda(setValueFunc, [] {});
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] {
        QList<IPHistroyData> dataList;
        IPHistroyData data1("smb://1.0.0.0", QDateTime::currentDateTime());
        IPHistroyData data2("ftp://1.0.0.0", QDateTime::currentDateTime());

        dataList << data1 << data2;
        return dataList;
    });

    EXPECT_NO_FATAL_FAILURE(SearchHistroyManager::instance()->writeIntoIPHistory("smb://1.0.0.0"));
    EXPECT_NO_FATAL_FAILURE(SearchHistroyManager::instance()->writeIntoIPHistory("smb://1.0.0.1"));
}
