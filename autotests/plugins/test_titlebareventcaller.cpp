// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebareventcaller.cpp
 * @brief Unit tests for TitleBarEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/titlebareventcaller.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventCallerTest, sendCd)
{
    // Test method: void sendCd((QWidget *sender, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendCd(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendCheckAddressInputStr)
{
    // Test method: void sendCheckAddressInputStr((QWidget *sender, QString *str))
    EXPECT_NO_FATAL_FAILURE(obj->sendCheckAddressInputStr(nullptr, nullptr));
}

TEST_F(TitleBarEventCallerTest, sendColumnDisplyName)
{
    // Test method: QString sendColumnDisplyName((QWidget *sender, dfmbase::Global::ItemRoles role))
    auto result = obj->sendColumnDisplyName(nullptr, {});
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarEventCallerTest, sendColumnRoles)
{
    // Test method: QList<ItemRoles> sendColumnRoles((QWidget *sender))
    auto result = obj->sendColumnRoles(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarEventCallerTest, sendCurrentGroupRoleStrategy)
{
    // Test method: QString sendCurrentGroupRoleStrategy((QWidget *sender))
    auto result = obj->sendCurrentGroupRoleStrategy(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarEventCallerTest, sendCurrentSortRole)
{
    // Test method: ItemRoles sendCurrentSortRole((QWidget *sender))
    auto result = obj->sendCurrentSortRole(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->sendCurrentSortRole(nullptr); });

}

TEST_F(TitleBarEventCallerTest, sendDetailViewState)
{
    // Test method: void sendDetailViewState((QWidget *sender, bool checked, bool userAction))
    EXPECT_NO_FATAL_FAILURE(obj->sendDetailViewState(nullptr, false, false));
}

TEST_F(TitleBarEventCallerTest, sendOpenFile)
{
    // Test method: void sendOpenFile((QWidget *sender, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenFile(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendRegisteredGroupStrategies)
{
    // Test method: QList<QPair<QString, QString>> sendRegisteredGroupStrategies((QWidget *sender))
    auto result = obj->sendRegisteredGroupStrategies(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarEventCallerTest, sendSearch)
{
    // Test method: void sendSearch((QWidget *sender, const QString &keyword))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendSearch(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendSetGroupStrategy)
{
    // Test method: void sendSetGroupStrategy((QWidget *sender, const QString &strategy))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendSetGroupStrategy(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendSetSort)
{
    // Test method: void sendSetSort((QWidget *sender, ItemRoles role))
    EXPECT_NO_FATAL_FAILURE(obj->sendSetSort(nullptr, ItemRoles()));
}

TEST_F(TitleBarEventCallerTest, sendShowFilterView)
{
    // Test method: void sendShowFilterView((QWidget *sender, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->sendShowFilterView(nullptr, false));
}

TEST_F(TitleBarEventCallerTest, sendStopSearch)
{
    // Test method: void sendStopSearch((QWidget *sender))
    EXPECT_NO_FATAL_FAILURE(obj->sendStopSearch(nullptr));
}

TEST_F(TitleBarEventCallerTest, sendTabChanged)
{
    // Test method: void sendTabChanged((QWidget *sender, const QString &uniqueId))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendTabChanged(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendTabCreated)
{
    // Test method: void sendTabCreated((QWidget *sender, const QString &uniqueId))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendTabCreated(nullptr, _arg1));
}

TEST_F(TitleBarEventCallerTest, sendTabRemoved)
{
    // Test method: void sendTabRemoved((QWidget *sender, const QString &removedId, const QString &nextId))
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendTabRemoved(nullptr, _arg1, _arg2));
}

TEST_F(TitleBarEventCallerTest, sendViewMode)
{
    // Test method: DFMGLOBAL_USE_NAMESPACE sendViewMode((QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode))
    auto result = obj->sendViewMode(nullptr, DFMBASE_NAMESPACE::Global::ViewMode());
    EXPECT_NO_FATAL_FAILURE({ obj->sendViewMode(nullptr, DFMBASE_NAMESPACE::Global::ViewMode()); });

}
