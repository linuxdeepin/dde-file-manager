// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/mountfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestMountFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMountFileInfo";
        info = new MountFileInfo(DUrl("/"));
    }

    void TearDown() override
    {
        std::cout << "end TestMountFileInfo";
        delete info;
    }

public:
    MountFileInfo *info;
};
} // namespace

TEST_F(TestMountFileInfo, canFetch)
{
    EXPECT_TRUE(info->canFetch());
}
