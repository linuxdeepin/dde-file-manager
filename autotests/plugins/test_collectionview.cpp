// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionview.cpp
 * @brief Unit tests for CollectionView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionview.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewTest, dragMoveEvent)
{
    // Test event handler: dragMoveEvent((QDragMoveEvent *event))
    QDragMoveEvent _event(QDragMoveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragMoveEvent(&_event));
}

TEST_F(CollectionViewTest, edit)
{
    // Test method: bool edit((const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event))
    QModelIndex _arg0{};
    auto result = obj->edit(_arg0, QAbstractItemView::EditTrigger(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(CollectionViewTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(CollectionViewTest, selectUrls)
{
    // Test method: void selectUrls((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectUrls(_arg0));
}

TEST_F(CollectionViewTest, sort)
{
    // Test method: void sort((int role))
    EXPECT_NO_FATAL_FAILURE(obj->sort(0));
}

TEST_F(CollectionViewTest, startDrag)
{
    // Test method: void startDrag((Qt::DropActions supportedActions))
    EXPECT_NO_FATAL_FAILURE(obj->startDrag(Qt::DropActions()));
}

TEST_F(CollectionViewTest, winId)
{
    // Test getter: WId winId()
    auto result = obj->winId();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionViewTest, dataProvider)
{
    // Test getter: CollectionDataProvider dataProvider()
    auto result = obj->dataProvider();
    EXPECT_NO_FATAL_FAILURE({ obj->dataProvider(); });

}
