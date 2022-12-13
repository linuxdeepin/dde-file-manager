/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "fileinfo/searchfileinfo.h"
#include "utils/searchhelper.h"

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE

TEST(SearchFileInfoTest, ut_exists)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_TRUE(info.exists());
}

TEST(SearchFileInfoTest, ut_isHidden)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsHidden));
}

TEST(SearchFileInfoTest, ut_isReadable)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsReadable));
}

TEST(SearchFileInfoTest, ut_isWritable)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsWritable));
}

TEST(SearchFileInfoTest, ut_isDir)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsDir));
}

TEST(SearchFileInfoTest, ut_size)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_EQ(-1, info.size());
}

TEST(SearchFileInfoTest, ut_emptyDirectoryTip)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_EQ("No results", info.viewOfTip(ViewInfoType::kEmptyDir));
}

TEST(SearchFileInfoTest, ut_loadingTip)
{
    SearchFileInfo info(SearchHelper::rootUrl());
    EXPECT_EQ("Searching...", info.viewOfTip(ViewInfoType::kLoading));
}
