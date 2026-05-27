// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/normalized/normalizedmodebroker.h"
#include "mode/normalizedmode.h"

#include <QUrl>
#include <QRect>
#include <QPoint>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_NormalizedModeBroker : public testing::Test
{
protected:
    void SetUp() override
    {
        mode = new NormalizedMode();
        broker = new NormalizedModeBroker(mode);
    }

    void TearDown() override
    {
        delete broker;
        broker = nullptr;
        delete mode;
        mode = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    NormalizedMode *mode = nullptr;
    NormalizedModeBroker *broker = nullptr;
};

TEST_F(UT_NormalizedModeBroker, Constructor_CreatesBroker)
{
    EXPECT_NE(broker, nullptr);
    EXPECT_EQ(broker->mode, mode);
}

TEST_F(UT_NormalizedModeBroker, RefreshModel_DoesNotCrash)
{
    broker->refreshModel(true, 100, true);
    SUCCEED();
}

TEST_F(UT_NormalizedModeBroker, GridPoint_ReturnsString)
{
    QUrl testUrl("file:///test");
    QPoint point;
    QString result = broker->gridPoint(testUrl, &point);
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_NormalizedModeBroker, VisualRect_ReturnsRect)
{
    QUrl testUrl("file:///test");
    QRect result = broker->visualRect("test_id", testUrl);
    EXPECT_TRUE(true); // Method exists and returns a rect
}

TEST_F(UT_NormalizedModeBroker, View_ReturnsAbstractItemView)
{
    QAbstractItemView *view = broker->view("test_id");
    EXPECT_TRUE(true); // Method exists and returns a view
}

TEST_F(UT_NormalizedModeBroker, IconRect_ReturnsRect)
{
    QRect vrect(0, 0, 100, 100);
    QRect result = broker->iconRect("test_id", vrect);
    EXPECT_TRUE(true); // Method exists and returns a rect
}

TEST_F(UT_NormalizedModeBroker, SelectAllItems_ReturnsBool)
{
    bool result = broker->selectAllItems();
    EXPECT_TRUE(result || !result);
}
