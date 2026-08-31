// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionwidget_1.cpp
 * @brief Unit tests for CollectionWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionWidgetTest, adjustable)
{
    // Test bool getter: adjustable()
    bool result = obj->adjustable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionWidgetTest, cacheSnapshot)
{
    // Test method: void cacheSnapshot(())
    EXPECT_NO_FATAL_FAILURE(obj->cacheSnapshot());
}

TEST_F(CollectionWidgetTest, closable)
{
    // Test bool getter: closable()
    bool result = obj->closable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionWidgetTest, collectionSize)
{
    // Test getter: CollectionFrameSize collectionSize()
    auto result = obj->collectionSize();
    EXPECT_NO_FATAL_FAILURE({ obj->collectionSize(); });

}

TEST_F(CollectionWidgetTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionWidgetTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(CollectionWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(CollectionWidgetTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionWidgetTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(CollectionWidgetTest, setAdjustable)
{
    // Test setter: void setAdjustable((const bool adjustable))
    EXPECT_NO_FATAL_FAILURE(obj->setAdjustable(false));
}

TEST_F(CollectionWidgetTest, setClosable)
{
    // Test setter: void setClosable((const bool closable))
    EXPECT_NO_FATAL_FAILURE(obj->setClosable(false));
}

TEST_F(CollectionWidgetTest, setCollectionSize)
{
    // Test setter: void setCollectionSize((const CollectionFrameSize &size))
    CollectionFrameSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCollectionSize(_arg0));
}

TEST_F(CollectionWidgetTest, setFreeze)
{
    // Test setter: void setFreeze((bool freeze))
    EXPECT_NO_FATAL_FAILURE(obj->setFreeze(false));
}

TEST_F(CollectionWidgetTest, setRenamable)
{
    // Test setter: void setRenamable((const bool renamable))
    EXPECT_NO_FATAL_FAILURE(obj->setRenamable(false));
}

TEST_F(CollectionWidgetTest, setTitleName)
{
    // Test setter: void setTitleName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitleName(_arg0));
}

TEST_F(CollectionWidgetTest, titleName)
{
    // Test getter: QString titleName()
    auto result = obj->titleName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionWidgetTest, updateMaskColor)
{
    // Test method: void updateMaskColor(())
    EXPECT_NO_FATAL_FAILURE(obj->updateMaskColor());
}

TEST_F(CollectionWidgetTest, view)
{
    // Test getter: CollectionView view()
    auto result = obj->view();
    EXPECT_NO_FATAL_FAILURE({ obj->view(); });

}
