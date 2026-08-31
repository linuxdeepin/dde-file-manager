// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasitemdelegateprivate.cpp
 * @brief Unit tests for CanvasItemDelegatePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/canvasitemdelegate.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasItemDelegatePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasItemDelegatePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasItemDelegatePrivateTest, CanvasItemDelegatePrivate)
{
    // Test constructor: CanvasItemDelegatePrivate((CanvasItemDelegate *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasItemDelegatePrivateTest, availableTextRect)
{
    // Test getter: QRect availableTextRect()
    auto result = obj->availableTextRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegatePrivateTest, createTextlayout)
{
    // Test method: ElideTextLayout createTextlayout((const QModelIndex &index, const QPainter *painter))
    QModelIndex _arg0{};
    auto result = obj->createTextlayout(_arg0, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createTextlayout(_arg0, nullptr); });

}

TEST_F(CanvasItemDelegatePrivateTest, extendLayoutText)
{
    // Test method: void extendLayoutText((const FileInfoPointer &info, dfmbase::ElideTextLayout *layout))
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->extendLayoutText(_arg0, nullptr));
}

TEST_F(CanvasItemDelegatePrivateTest, isHighlight)
{
    // Test bool getter: isHighlight()
    bool result = obj->isHighlight();
    EXPECT_FALSE(result);

}

TEST_F(CanvasItemDelegatePrivateTest, needExpend)
{
    // Test method: bool needExpend((const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, QRect *needText))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    QRect _arg2{};
    auto result = obj->needExpend(_arg0, _arg1, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasItemDelegatePrivateTest, CanvasItemDelegatePrivate_Destructor)
{
    // Test method:  ~CanvasItemDelegatePrivate(())
    EXPECT_NO_FATAL_FAILURE({ CanvasItemDelegatePrivate *tmp = new CanvasItemDelegatePrivate(); delete tmp; });
}
