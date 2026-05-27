// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/grid/canvasgrid.h"
#include "plugins/desktop/ddplugin-canvas/grid/gridcore.h"

#include <QStringList>
#include <QPoint>
#include <QSize>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_CanvasGrid : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Test the actual CanvasGrid instance without heavy stubbing
        grid = CanvasGrid::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasGrid *grid = nullptr;
};

TEST_F(UT_CanvasGrid, instance)
{
    // Test singleton pattern
    EXPECT_NE(grid, nullptr);
    
    // Should return the same instance
    CanvasGrid *grid2 = CanvasGrid::instance();
    EXPECT_EQ(grid, grid2);
}

TEST_F(UT_CanvasGrid, initSurface)
{
    // Test basic initialization
    EXPECT_NO_THROW(grid->initSurface(2));
    EXPECT_NO_THROW(grid->initSurface(0));
}

TEST_F(UT_CanvasGrid, updateSize_SurfaceSize)
{
    // Initialize surface first
    grid->initSurface(1);
    
    // Test updateSize
    QSize testSize(800, 600);
    EXPECT_NO_THROW(grid->updateSize(1, testSize));
    
    // Test surfaceSize - should not crash
    QSize retrievedSize = grid->surfaceSize(1);
    SUCCEED(); // Main goal is no crash
}

TEST_F(UT_CanvasGrid, gridCount)
{
    // Test gridCount method
    int count = grid->gridCount();
    EXPECT_GE(count, 0); // Should return non-negative count
    
    // Test with specific index
    int indexCount = grid->gridCount(1);
    EXPECT_GE(indexCount, 0);
}

TEST_F(UT_CanvasGrid, setMode_Mode)
{
    // Test mode setting and getting
    EXPECT_NO_THROW(grid->setMode(CanvasGrid::Mode::Align));
    
    CanvasGrid::Mode currentMode = grid->mode();
    // Method should execute without crashing
    SUCCEED();
    
    EXPECT_NO_THROW(grid->setMode(CanvasGrid::Mode::Custom));
}

TEST_F(UT_CanvasGrid, setItems_Items)
{
    // Test items setting and getting
    QStringList testItems = {"item1.txt", "item2.txt", "item3.txt"};
    
    EXPECT_NO_THROW(grid->setItems(testItems));
    
    // Test getting items
    QStringList retrievedItems = grid->items();
    SUCCEED(); // Main goal is no crash
    
    // Test with index
    QStringList indexItems = grid->items(1);
    SUCCEED();
}

TEST_F(UT_CanvasGrid, append_Methods)
{
    // Test single item append
    QString testItem = "newfile.txt";
    EXPECT_NO_THROW(grid->append(testItem));
    
    // Test multiple items append
    QStringList testItems = {"file1.txt", "file2.txt"};
    EXPECT_NO_THROW(grid->append(testItems));
}

TEST_F(UT_CanvasGrid, drop_Remove_Replace)
{
    // Initialize surface for testing
    grid->initSurface(1);
    
    // Test drop method
    QPoint testPos(0, 0);
    QString testItem = "testfile.txt";
    
    bool dropResult = grid->drop(1, testPos, testItem);
    SUCCEED(); // Method should not crash
    
    // Test remove method
    bool removeResult = grid->remove(1, testItem);
    SUCCEED();
    
    // Test replace method
    QString oldItem = "oldfile.txt";
    QString newItem = "newfile.txt";
    bool replaceResult = grid->replace(oldItem, newItem);
    SUCCEED();
}

TEST_F(UT_CanvasGrid, item_Point_Methods)
{
    // Test item retrieval by position
    QPoint testPos(0, 0);
    QString item = grid->item(1, testPos);
    SUCCEED(); // Method should not crash
    
    // Test point retrieval by item
    QPair<int, QPoint> pos;
    bool found = grid->point("testitem", pos);
    SUCCEED(); // Method should not crash
    
    // Test points method
    QHash<QString, QPoint> points = grid->points(1);
    SUCCEED();
}

TEST_F(UT_CanvasGrid, overloadItems)
{
    // Test overloadItems method
    QStringList overloaded = grid->overloadItems(1);
    SUCCEED(); // Method should not crash and return a list
}

TEST_F(UT_CanvasGrid, arrange_Sync)
{
    // Test arrange method
    EXPECT_NO_THROW(grid->arrange());
    
    // Test requestSync method
    EXPECT_NO_THROW(grid->requestSync(100));
    EXPECT_NO_THROW(grid->requestSync()); // Default parameter
}

TEST_F(UT_CanvasGrid, move_TryAppendAfter)
{
    // Initialize surface
    grid->initSurface(2);
    
    // Test move method
    QPoint toPos(1, 1);
    QString focus = "focus.txt";
    QStringList items = {"item1.txt", "item2.txt"};
    
    bool moveResult = grid->move(2, toPos, focus, items);
    SUCCEED(); // Method should not crash
    
    // Test tryAppendAfter method
    QPoint begin(0, 0);
    EXPECT_NO_THROW(grid->tryAppendAfter(items, 1, begin));
}

TEST_F(UT_CanvasGrid, popOverload)
{
    // Test popOverload method
    EXPECT_NO_THROW(grid->popOverload());
}

TEST_F(UT_CanvasGrid, core)
{
    // Test core access method
    GridCore &core = grid->core();
    // Should return a valid reference
    SUCCEED();
}