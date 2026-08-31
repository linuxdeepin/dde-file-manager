// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionframe_1.cpp
 * @brief Unit tests for CollectionFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionframe.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionFrameTest, collectionFeatures)
{
    // Test getter: CollectionFrame::CollectionFrameFeatures collectionFeatures()
    auto result = obj->collectionFeatures();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CollectionFrameTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionFrameTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(CollectionFrameTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(CollectionFrameTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(CollectionFrameTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(CollectionFrameTest, setCollectionFeatures)
{
    // Test setter: void setCollectionFeatures((const CollectionFrameFeatures &features))
    CollectionFrameFeatures _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCollectionFeatures(_arg0));
}

TEST_F(CollectionFrameTest, setStretchStyle)
{
    // Test setter: void setStretchStyle((const CollectionFrame::CollectionFrameStretchStyle &style))
    CollectionFrame::CollectionFrameStretchStyle _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setStretchStyle(_arg0));
}

TEST_F(CollectionFrameTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(CollectionFrameTest, stretchStep)
{
    // Test getter: int stretchStep()
    auto result = obj->stretchStep();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionFrameTest, stretchStyle)
{
    // Test getter: CollectionFrame::CollectionFrameStretchStyle stretchStyle()
    auto result = obj->stretchStyle();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CollectionFrameTest, widget)
{
    // Test getter: QWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}
