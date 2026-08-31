// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_policykithelper.cpp
 * @brief Unit tests for PolicyKitHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/common/polkit/policykithelper.h"

#include <QTest>

using namespace src;

class PolicyKitHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PolicyKitHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PolicyKitHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PolicyKitHelperTest, PolicyKitHelper)
{
    // Test constructor: PolicyKitHelper(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(PolicyKitHelperTest, checkAuthorization)
{
    // Test method: bool checkAuthorization((const QString &actionId, const QString &appBusName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkAuthorization(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PolicyKitHelperTest, PolicyKitHelper_Destructor)
{
    // Test method:  ~PolicyKitHelper(())
    EXPECT_NO_FATAL_FAILURE({ PolicyKitHelper *tmp = new PolicyKitHelper(); delete tmp; });
}
