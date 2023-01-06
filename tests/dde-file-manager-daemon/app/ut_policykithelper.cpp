// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "app/policykithelper.h"

namespace {
class TestPolicyKitHelper : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestPolicyKitHelper";
    }

    void TearDown() override
    {
        std::cout << "end TestPolicyKitHelper";
    }

public:
};
} // namespace

TEST_F(TestPolicyKitHelper, coverageTest)
{
    PolicyKitHelper::instance()->checkAuthorization("", 0);
}
