// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dragdrophelper_1.cpp
 * @brief Unit tests for DragDropHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/dragdrophelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class DragDropHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DragDropHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DragDropHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DragDropHelperTest, DragDropHelper)
{
    // Test constructor: DragDropHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DragDropHelperTest, checkAction)
{
    // Test method: Qt::DropAction checkAction((Qt::DropAction srcAction, bool sameUser))
    auto result = obj->checkAction(Qt::DropAction(), false);
    EXPECT_NO_FATAL_FAILURE({ obj->checkAction(Qt::DropAction(), false); });

}

TEST_F(DragDropHelperTest, checkDragEnable)
{
    // Test method: bool checkDragEnable((const QUrl &dragUrl, const QUrl &targetUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->checkDragEnable(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, checkMoveEnable)
{
    // Test method: bool checkMoveEnable((const QUrl &dragUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->checkMoveEnable(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, checkProhibitPaths)
{
    // Test method: bool checkProhibitPaths((QDragEnterEvent *event, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    auto result = obj->checkProhibitPaths(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, checkTargetEnable)
{
    // Test method: bool checkTargetEnable((const QUrl &targetUrl))
    QUrl _arg0{};
    auto result = obj->checkTargetEnable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, dragLeave)
{
    // Test method: bool dragLeave((QDragLeaveEvent *event))
    auto result = obj->dragLeave(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, fileInfoAtPos)
{
    // Test method: QSharedPointer<FileInfo> fileInfoAtPos((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->fileInfoAtPos(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DragDropHelperTest, handleDFileDrag)
{
    // Test method: bool handleDFileDrag((const QMimeData *data, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->handleDFileDrag(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, isDragTarget)
{
    // Test method: bool isDragTarget((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isDragTarget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DragDropHelperTest, timerEvent)
{
    // Test event handler: timerEvent((QTimerEvent *event))
    QTimerEvent _event(QTimerEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->timerEvent(&_event));
}

TEST_F(DragDropHelperTest, updateDragAutoScroll)
{
    // Test method: void updateDragAutoScroll((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateDragAutoScroll(_arg0));
}

TEST_F(DragDropHelperTest, updateDragHighlight)
{
    // Test method: void updateDragHighlight((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateDragHighlight(_arg0));
}
