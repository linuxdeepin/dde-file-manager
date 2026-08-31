// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_stickygroupheaderhelper_1.cpp
 * @brief Unit tests for StickyGroupHeaderHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/stickygroupheaderhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class StickyGroupHeaderHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new StickyGroupHeaderHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    StickyGroupHeaderHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(StickyGroupHeaderHelperTest, StickyGroupHeaderHelper)
{
    // Test constructor: StickyGroupHeaderHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(StickyGroupHeaderHelperTest, cachedNextStickyHeader)
{
    // Test getter: QModelIndex cachedNextStickyHeader()
    auto result = obj->cachedNextStickyHeader();
    EXPECT_FALSE(result.isValid());

}

TEST_F(StickyGroupHeaderHelperTest, computeStickyY)
{
    // Test method: int computeStickyY((int headerHeight))
    auto result = obj->computeStickyY(0);
    EXPECT_GE(result, 0);

}

TEST_F(StickyGroupHeaderHelperTest, currentStickyIndex)
{
    // Test getter: QModelIndex currentStickyIndex()
    auto result = obj->currentStickyIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(StickyGroupHeaderHelperTest, currentStickyRect)
{
    // Test getter: QRect currentStickyRect()
    auto result = obj->currentStickyRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(StickyGroupHeaderHelperTest, groupHeaderContentTop)
{
    // Test method: int groupHeaderContentTop((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->groupHeaderContentTop(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(StickyGroupHeaderHelperTest, isPosInStickyHeader)
{
    // Test method: bool isPosInStickyHeader((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->isPosInStickyHeader(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(StickyGroupHeaderHelperTest, isStickyHeaderHovered)
{
    // Test bool getter: isStickyHeaderHovered()
    bool result = obj->isStickyHeaderHovered();
    EXPECT_FALSE(result);

}

TEST_F(StickyGroupHeaderHelperTest, paintStickyHeaderOverlay)
{
    // Test method: void paintStickyHeaderOverlay((const QModelIndex &index, int y, int headerHeight))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->paintStickyHeaderOverlay(_arg0, 0, 0));
}

TEST_F(StickyGroupHeaderHelperTest, scrollStickyHeaderToTop)
{
    // Test method: void scrollStickyHeaderToTop((const QModelIndex &headerIndex))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scrollStickyHeaderToTop(_arg0));
}

TEST_F(StickyGroupHeaderHelperTest, setCachedNextStickyHeader)
{
    // Test method: void setCachedNextStickyHeader(())
    EXPECT_NO_FATAL_FAILURE(obj->setCachedNextStickyHeader());
}

TEST_F(StickyGroupHeaderHelperTest, setCurrentStickyIndex)
{
    // Test method: void setCurrentStickyIndex(())
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentStickyIndex());
}

TEST_F(StickyGroupHeaderHelperTest, setCurrentStickyRect)
{
    // Test method: void setCurrentStickyRect(())
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentStickyRect());
}

TEST_F(StickyGroupHeaderHelperTest, setStickyHeaderHovered)
{
    // Test method: void setStickyHeaderHovered(())
    EXPECT_NO_FATAL_FAILURE(obj->setStickyHeaderHovered());
}

TEST_F(StickyGroupHeaderHelperTest, stickyHeaderHeight)
{
    // Test getter: int stickyHeaderHeight()
    auto result = obj->stickyHeaderHeight();
    EXPECT_EQ(result, 0);

}
