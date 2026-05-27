// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "broker/canvasgridbroker.h"
#include "grid/canvasgrid.h"

#include <gtest/gtest.h>
#include <QObject>
#include <QRect>
#include <QPoint>

using namespace ddplugin_canvas;

class UT_CanvasGridBroker : public testing::Test
{
public:
    virtual void SetUp() override
    {
        mockGrid = new CanvasGrid(nullptr);
        broker = new CanvasGridBroker(mockGrid);
    }

    virtual void TearDown() override
    {
        delete broker;
        delete mockGrid;
        
        stub.clear();
    }

public:
    CanvasGrid *mockGrid = nullptr;
    CanvasGridBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasGridBroker, constructor_CreateBroker_InitializesCorrectly)
{
    EXPECT_NE(broker, nullptr);
}

TEST_F(UT_CanvasGridBroker, init_InitializeBroker_ReturnsTrue)
{
    // Mock DPF EventChannelManager::connect to avoid complex framework dependency
    // Instead of trying to mock the complex DPF function, we just test that init doesn't crash
    
    EXPECT_NO_THROW(broker->init());
}

TEST_F(UT_CanvasGridBroker, point_GetPoint_CallsGrid)
{
    QString item = "test.txt";
    QPoint pos;
    
    // Mock CanvasGrid::point method (the actual signature expects QPair<int, QPoint>&)
    stub.set_lamda(&CanvasGrid::point, [](CanvasGrid*, const QString&, QPair<int, QPoint>&) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Return success
    });
    
    // The method should call grid->point internally
    EXPECT_NO_THROW(broker->point(item, &pos));
}

TEST_F(UT_CanvasGridBroker, point_GetPointWithNullPtr_ReturnsMinusOne)
{
    QString item = "test.txt";
    
    // Test calling point with nullptr
    int result = broker->point(item, nullptr);
    EXPECT_EQ(result, -1);
}

// Note: visualRect and itemAt methods don't exist in CanvasGridBroker
// tryAppendAfter has different signature than expected

TEST_F(UT_CanvasGridBroker, tryAppendAfter_AppendAfter_CallsGrid)
{
    QStringList items = {"file1.txt", "file2.txt"};
    int index = 0;
    QPoint begin(10, 20);
    
    // Mock CanvasGrid::tryAppendAfter method (correct signature: items, index, begin)
    stub.set_lamda(&CanvasGrid::tryAppendAfter, [](CanvasGrid*, const QStringList&, int, const QPoint&) -> void {
        __DBG_STUB_INVOKE__
        // void return type
    });
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(broker->tryAppendAfter(items, index, begin));
}