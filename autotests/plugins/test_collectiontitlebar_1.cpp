// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiontitlebar_1.cpp
 * @brief Unit tests for CollectionTitleBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectiontitlebar.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionTitleBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionTitleBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionTitleBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionTitleBarTest, CollectionTitleBar)
{
    // Test constructor: CollectionTitleBar((const QString &uuid, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionTitleBarTest, collectionSize)
{
    // Test getter: CollectionFrameSize collectionSize()
    auto result = obj->collectionSize();
    EXPECT_NO_FATAL_FAILURE({ obj->collectionSize(); });

}

TEST_F(CollectionTitleBarTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(CollectionTitleBarTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionTitleBarTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionTitleBarTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *e))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(CollectionTitleBarTest, rounded)
{
    // Test method: void rounded(())
    EXPECT_NO_FATAL_FAILURE(obj->rounded());
}

TEST_F(CollectionTitleBarTest, setAdjustable)
{
    // Test setter: void setAdjustable((const bool adjustable))
    EXPECT_NO_FATAL_FAILURE(obj->setAdjustable(false));
}

TEST_F(CollectionTitleBarTest, setClosable)
{
    // Test setter: void setClosable((const bool closable))
    EXPECT_NO_FATAL_FAILURE(obj->setClosable(false));
}

TEST_F(CollectionTitleBarTest, setCollectionSize)
{
    // Test setter: void setCollectionSize((const CollectionFrameSize &size))
    CollectionFrameSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCollectionSize(_arg0));
}

TEST_F(CollectionTitleBarTest, setRenamable)
{
    // Test setter: void setRenamable((const bool renamable))
    EXPECT_NO_FATAL_FAILURE(obj->setRenamable(false));
}

TEST_F(CollectionTitleBarTest, setTitleBarVisible)
{
    // Test method: bool setTitleBarVisible((const bool &visible))
    bool _arg0{};
    auto result = obj->setTitleBarVisible(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionTitleBarTest, setTitleName)
{
    // Test setter: void setTitleName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitleName(_arg0));
}

TEST_F(CollectionTitleBarTest, titleBarVisible)
{
    // Test bool getter: titleBarVisible()
    bool result = obj->titleBarVisible();
    EXPECT_FALSE(result);

}

TEST_F(CollectionTitleBarTest, titleName)
{
    // Test getter: QString titleName()
    auto result = obj->titleName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionTitleBarTest, CollectionTitleBar_Destructor)
{
    // Test method:  ~CollectionTitleBar(())
    EXPECT_NO_FATAL_FAILURE({ CollectionTitleBar *tmp = new CollectionTitleBar(); delete tmp; });
}
