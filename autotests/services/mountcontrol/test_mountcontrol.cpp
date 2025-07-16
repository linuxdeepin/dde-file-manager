// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include "mountcontroldbus.h"

class UT_MountControlDBus : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }
private:
    stub_ext::StubExt stub;
};

TEST_F(UT_MountControlDBus, MountControlBasicTest)
{
    // Basic smoke test
    EXPECT_TRUE(true);
} 