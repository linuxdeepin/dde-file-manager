// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewprivate.cpp
 * @brief Unit tests for CanvasViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/canvasview.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewPrivateTest, findIndex)
{
    // Test method: QModelIndex findIndex((const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent))
    QString _arg0{};
    QModelIndex _arg2{};
    auto result = obj->findIndex(_arg0, false, _arg2, false, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewPrivateTest, firstIndex)
{
    // Test getter: QModelIndex firstIndex()
    auto result = obj->firstIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewPrivateTest, lastIndex)
{
    // Test getter: QModelIndex lastIndex()
    auto result = obj->lastIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewPrivateTest, updateGridSize)
{
    // Test method: void updateGridSize((const QSize &viewSize, const QMargins &geometryMargins, const QSize &itemSize))
    QSize _arg0{};
    QMargins _arg1{};
    QSize _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateGridSize(_arg0, _arg1, _arg2));
}
