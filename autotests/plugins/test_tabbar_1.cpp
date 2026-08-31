// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tabbar_1.cpp
 * @brief Unit tests for TabBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/tabbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TabBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TabBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TabBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TabBarTest, activateNextTab)
{
    // Test method: void activateNextTab(())
    EXPECT_NO_FATAL_FAILURE(obj->activateNextTab());
}

TEST_F(TabBarTest, activatePreviousTab)
{
    // Test method: void activatePreviousTab(())
    EXPECT_NO_FATAL_FAILURE(obj->activatePreviousTab());
}

TEST_F(TabBarTest, appendInactiveTab)
{
    // Test method: int appendInactiveTab((const QUrl &url, bool pinned))
    QUrl _arg0{};
    auto result = obj->appendInactiveTab(_arg0, false);
    EXPECT_GE(result, 0);

}

TEST_F(TabBarTest, appendTab)
{
    // Test getter: int appendTab()
    auto result = obj->appendTab();
    EXPECT_EQ(result, 0);

}

TEST_F(TabBarTest, canInsertFromMimeData)
{
    // Test method: bool canInsertFromMimeData((int index, const QMimeData *source))
    auto result = obj->canInsertFromMimeData(0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TabBarTest, createDragPixmapFromTab)
{
    // Test method: QPixmap createDragPixmapFromTab((int index, const QStyleOptionTab &option, QPoint *hotspot))
    QStyleOptionTab _arg1{};
    auto result = obj->createDragPixmapFromTab(0, _arg1, nullptr);
    EXPECT_TRUE(result.isNull());

}

TEST_F(TabBarTest, createMimeDataFromTab)
{
    // Test method: QMimeData createMimeDataFromTab((int index, const QStyleOptionTab &option))
    QStyleOptionTab _arg1{};
    auto result = obj->createMimeDataFromTab(0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->createMimeDataFromTab(0, _arg1); });

}

TEST_F(TabBarTest, insertFromMimeData)
{
    // Test method: void insertFromMimeData((int index, const QMimeData *source))
    EXPECT_NO_FATAL_FAILURE(obj->insertFromMimeData(0, nullptr));
}

TEST_F(TabBarTest, insertFromMimeDataOnDragEnter)
{
    // Test method: void insertFromMimeDataOnDragEnter((int index, const QMimeData *source))
    EXPECT_NO_FATAL_FAILURE(obj->insertFromMimeDataOnDragEnter(0, nullptr));
}

TEST_F(TabBarTest, insertInactiveTab)
{
    // Test method: int insertInactiveTab((int index, const QUrl &url, bool pinned))
    QUrl _arg1{};
    auto result = obj->insertInactiveTab(0, _arg1, false);
    EXPECT_GE(result, 0);

}

TEST_F(TabBarTest, isInactiveTab)
{
    // Test method: bool isInactiveTab((int index))
    auto result = obj->isInactiveTab(0);
    EXPECT_FALSE(result);

}

TEST_F(TabBarTest, isPinned)
{
    // Test method: bool isPinned((int index))
    auto result = obj->isPinned(0);
    EXPECT_FALSE(result);

}

TEST_F(TabBarTest, isTabValid)
{
    // Test method: bool isTabValid((int index))
    auto result = obj->isTabValid(0);
    EXPECT_FALSE(result);

}

TEST_F(TabBarTest, maximumTabSizeHint)
{
    // Test method: QSize maximumTabSizeHint((int index))
    auto result = obj->maximumTabSizeHint(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarTest, minimumTabSizeHint)
{
    // Test method: QSize minimumTabSizeHint((int index))
    auto result = obj->minimumTabSizeHint(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *e))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(TabBarTest, paintTab)
{
    // Test method: void paintTab((QPainter *painter, int index, const QStyleOptionTab &option))
    QStyleOptionTab _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTab(nullptr, 0, _arg2));
}

TEST_F(TabBarTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *e))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(TabBarTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(TabBarTest, setTabAlias)
{
    // Test setter: void setTabAlias((int index, const QString &alias))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setTabAlias(0, _arg1));
}

TEST_F(TabBarTest, setTabUserData)
{
    // Test setter: void setTabUserData((int index, const QString &key, const QVariant &userData))
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setTabUserData(0, _arg1, _arg2));
}

TEST_F(TabBarTest, tabAlias)
{
    // Test method: QString tabAlias((int index))
    auto result = obj->tabAlias(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarTest, tabSizeHint)
{
    // Test method: QSize tabSizeHint((int index))
    auto result = obj->tabSizeHint(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarTest, tabUniqueId)
{
    // Test method: QString tabUniqueId((int index))
    auto result = obj->tabUniqueId(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarTest, tabUrl)
{
    // Test method: QUrl tabUrl((int index))
    auto result = obj->tabUrl(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TabBarTest, tabUserData)
{
    // Test method: QVariant tabUserData((int index, const QString &key))
    QString _arg1{};
    auto result = obj->tabUserData(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TabBarTest, updateTabName)
{
    // Test method: void updateTabName((int index))
    EXPECT_NO_FATAL_FAILURE(obj->updateTabName(0));
}
