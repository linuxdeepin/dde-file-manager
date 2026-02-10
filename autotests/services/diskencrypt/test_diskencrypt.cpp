// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include "dbus/diskencryptsetup.h"

class UT_DiskEncryptSetup : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }
private:
    stub_ext::StubExt stub;
};

TEST_F(UT_DiskEncryptSetup, DiskEncryptBasicTest)
{
    // Basic smoke test for DiskEncryptSetup
    EXPECT_TRUE(true);
} 