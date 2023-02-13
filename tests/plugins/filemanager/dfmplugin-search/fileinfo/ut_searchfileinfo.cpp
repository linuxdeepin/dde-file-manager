// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
