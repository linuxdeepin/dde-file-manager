// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewdefines.cpp
 * @brief Unit tests for ViewDefines (viewdefines.cpp)
 */

#include <gtest/gtest.h>
#include <QVariantList>

#include <dfm-base/utils/viewdefines.h>

using namespace dfmbase;

TEST(ViewDefinesTest, IconSizeCountIsPositive)
{
    ViewDefines vd;
    EXPECT_GT(vd.iconSizeCount(), 0);
}

TEST(ViewDefinesTest, IconSizeByIndexReturnsMinForFirst)
{
    ViewDefines vd;
    EXPECT_EQ(vd.iconSize(0), 24);
}

TEST(ViewDefinesTest, IconSizeByIndexReturnsMaxForLast)
{
    ViewDefines vd;
    int last = vd.iconSizeCount() - 1;
    EXPECT_EQ(vd.iconSize(last), 512);
}

TEST(ViewDefinesTest, IndexOfIconSizeFound)
{
    ViewDefines vd;
    EXPECT_EQ(vd.indexOfIconSize(48), 3);
    EXPECT_EQ(vd.indexOfIconSize(999), -1);
}

TEST(ViewDefinesTest, GetIconSizeListSizeMatches)
{
    ViewDefines vd;
    EXPECT_EQ(vd.getIconSizeList().size(), vd.iconSizeCount());
}

TEST(ViewDefinesTest, IconGridDensityCountIsPositive)
{
    ViewDefines vd;
    EXPECT_GT(vd.iconGridDensityCount(), 0);
}

TEST(ViewDefinesTest, IconGridDensityByIndex)
{
    ViewDefines vd;
    EXPECT_EQ(vd.iconGridDensity(0), 60);
}

TEST(ViewDefinesTest, IndexOfIconGridDensity)
{
    ViewDefines vd;
    EXPECT_GE(vd.indexOfIconGridDensity(60), 0);
    EXPECT_EQ(vd.indexOfIconGridDensity(99999), -1);
}

TEST(ViewDefinesTest, GetIconGridDensityListSize)
{
    ViewDefines vd;
    EXPECT_EQ(vd.getIconGridDensityList().size(), vd.iconGridDensityCount());
}

TEST(ViewDefinesTest, ListHeightCountIsThree)
{
    ViewDefines vd;
    EXPECT_EQ(vd.listHeightCount(), 3);
}

TEST(ViewDefinesTest, ListHeightByIndex)
{
    ViewDefines vd;
    EXPECT_EQ(vd.listHeight(0), 24);
    EXPECT_EQ(vd.listHeight(1), 32);
    EXPECT_EQ(vd.listHeight(2), 48);
}

TEST(ViewDefinesTest, IndexOfListHeight)
{
    ViewDefines vd;
    EXPECT_EQ(vd.indexOfListHeight(32), 1);
    EXPECT_EQ(vd.indexOfListHeight(100), -1);
}

TEST(ViewDefinesTest, GetListHeightListSize)
{
    ViewDefines vd;
    EXPECT_EQ(vd.getListHeightList().size(), vd.listHeightCount());
}
