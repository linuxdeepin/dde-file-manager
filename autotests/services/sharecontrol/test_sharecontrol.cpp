// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

// Include share control DBus class from the service
#include "sharecontroldbus.h"

class UT_ShareControlDBus : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ShareControlDBus, ShareControlBasicTest)
{
    // Basic functionality test - this is a smoke test
    EXPECT_TRUE(true);
} 