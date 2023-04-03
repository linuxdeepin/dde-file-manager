// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/searchhistroymanager.h"

#include <dfm-base/base/application/settings.h>

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
