// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractsortfilter.cpp
 * @brief Unit tests for AbstractSortFilter (abstractsortfilter.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/abstractsortfilter.h>

#include <QDir>

using namespace dfmbase;

TEST(AbstractSortFilterTest, DefaultLessThanReturnsMinusOne)
{
    AbstractSortFilter filter;
    FileInfoPointer left, right;
    EXPECT_EQ(filter.lessThan(left, right, false, Global::kItemNameRole,
                              AbstractSortFilter::SortScenarios::kSortScenariosNormal), -1);
}

TEST(AbstractSortFilterTest, DefaultLessThanReturnsMinusOneForAllScenarios)
{
    AbstractSortFilter filter;
    FileInfoPointer left, right;
    EXPECT_EQ(filter.lessThan(left, right, true, Global::kItemFileSizeRole,
                              AbstractSortFilter::SortScenarios::kSortScenariosIteratorAddFile), -1);
    EXPECT_EQ(filter.lessThan(left, right, false, Global::kItemFileDisplayNameRole,
                              AbstractSortFilter::SortScenarios::kSortScenariosWatcherOther), -1);
}

TEST(AbstractSortFilterTest, DefaultCheckFiltersReturnsMinusOne)
{
    AbstractSortFilter filter;
    FileInfoPointer info;
    EXPECT_EQ(filter.checkFilters(info, QDir::NoFilter, QVariant()), -1);
}

TEST(AbstractSortFilterTest, ConstructAndDestructSafe)
{
    { AbstractSortFilter f; (void)f; }
    SUCCEED();
}
