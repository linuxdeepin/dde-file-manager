// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgrid_1.cpp
 * @brief Unit tests for CanvasGrid methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgrid.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGrid();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGrid *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridTest, CanvasGrid)
{
    // Test constructor: CanvasGrid((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasGridTest, drop)
{
    // Test method: bool drop((int index, const QPoint &pos, const QString &item))
    QPoint _arg1{};
    QString _arg2{};
    auto result = obj->drop(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridTest, gridCount)
{
    // Test method: int gridCount((int index))
    auto result = obj->gridCount(0);
    EXPECT_GE(result, 0);

}

TEST_F(CanvasGridTest, initSurface)
{
    // Test method: void initSurface((int count))
    EXPECT_NO_FATAL_FAILURE(obj->initSurface(0));
}

TEST_F(CanvasGridTest, mode)
{
    // Test getter: CanvasGrid::Mode mode()
    auto result = obj->mode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CanvasGridTest, overloadItems)
{
    // Test method: QStringList overloadItems((int index))
    auto result = obj->overloadItems(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridTest, point)
{
    // Test method: bool point((const QString &item, QPair<int, QPoint> &pos))
    QString _arg0{};
    QPair<int, QPoint> _arg1{};
    auto result = obj->point(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridTest, popOverload)
{
    // Test method: void popOverload(())
    EXPECT_NO_FATAL_FAILURE(obj->popOverload());
}

TEST_F(CanvasGridTest, requestSync)
{
    // Test method: void requestSync((int ms))
    EXPECT_NO_FATAL_FAILURE(obj->requestSync(0));
}

TEST_F(CanvasGridTest, setItems)
{
    // Test setter: void setItems((const QStringList &items))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setItems(_arg0));
}

TEST_F(CanvasGridTest, setMode)
{
    // Test setter: void setMode((CanvasGrid::Mode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setMode(CanvasGrid::Mode()));
}

TEST_F(CanvasGridTest, surfaceSize)
{
    // Test method: QSize surfaceSize((int index))
    auto result = obj->surfaceSize(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridTest, updateSize)
{
    // Test method: void updateSize((int index, const QSize &size))
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateSize(0, _arg1));
}

TEST_F(CanvasGridTest, CanvasGrid_Destructor)
{
    // Test method:  ~CanvasGrid(())
    EXPECT_NO_FATAL_FAILURE({ CanvasGrid *tmp = new CanvasGrid(); delete tmp; });
}
