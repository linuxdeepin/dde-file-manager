// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dragdropoper_1.cpp
 * @brief Unit tests for DragDropOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/dragdropoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DragDropOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DragDropOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DragDropOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DragDropOperTest, DragDropOper)
{
    // Test constructor: DragDropOper((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DragDropOperTest, checkProhibitPaths)
{
    // Test method: bool checkProhibitPaths((QDragEnterEvent *event))
    auto result = obj->checkProhibitPaths(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, checkSourceValid)
{
    // Test method: bool checkSourceValid((const QList<QUrl> &srcUrls))
    QList<QUrl> _arg0{};
    auto result = obj->checkSourceValid(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, checkTargetEnable)
{
    // Test method: bool checkTargetEnable((const QUrl &targetUrl))
    QUrl _arg0{};
    auto result = obj->checkTargetEnable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, dropClientDownload)
{
    // Test method: bool dropClientDownload((QDropEvent *event))
    auto result = obj->dropClientDownload(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, dropFilter)
{
    // Test method: bool dropFilter((QDropEvent *event))
    auto result = obj->dropFilter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropOperTest, handleMoveMimeData)
{
    // Test method: void handleMoveMimeData((QDropEvent *event, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMoveMimeData(nullptr, _arg1));
}

TEST_F(DragDropOperTest, leave)
{
    // Test method: void leave((QDragLeaveEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->leave(nullptr));
}

TEST_F(DragDropOperTest, stopDelayDodge)
{
    // Test method: void stopDelayDodge(())
    EXPECT_NO_FATAL_FAILURE(obj->stopDelayDodge());
}

TEST_F(DragDropOperTest, tryDodge)
{
    // Test method: void tryDodge((QDragMoveEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->tryDodge(nullptr));
}

TEST_F(DragDropOperTest, updateDFMMimeData)
{
    // Test method: void updateDFMMimeData((QDropEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->updateDFMMimeData(nullptr));
}

TEST_F(DragDropOperTest, updateDragHover)
{
    // Test method: void updateDragHover((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateDragHover(_arg0));
}

TEST_F(DragDropOperTest, updatePrepareDodgeValue)
{
    // Test method: void updatePrepareDodgeValue((QEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->updatePrepareDodgeValue(nullptr));
}

TEST_F(DragDropOperTest, updateTarget)
{
    // Test method: void updateTarget((const QMimeData *data, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateTarget(nullptr, _arg1));
}
