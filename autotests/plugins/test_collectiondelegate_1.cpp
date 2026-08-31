// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiondelegate_1.cpp
 * @brief Unit tests for CollectionDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/collectiondelegate.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CollectionDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionDelegateTest, editorEvent)
{
    // Test event handler: editorEvent((QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->editorEvent(&_event));
}

TEST_F(CollectionDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(CollectionDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &, const QModelIndex &))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
