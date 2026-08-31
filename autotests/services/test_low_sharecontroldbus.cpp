// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_sharecontroldbus.cpp
 * @brief Unit tests for ShareControlDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/sharecontrol/sharecontroldbus.h"

#include <QTest>

using namespace src;

class ShareControlDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareControlDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareControlDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareControlDBusTest, IsUserSharePasswordSet)
{
    // Test method: bool IsUserSharePasswordSet((const QString &username))
    QString _arg0{};
    auto result = obj->IsUserSharePasswordSet(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShareControlDBusTest, ShareControlDBus)
{
    // Test constructor: ShareControlDBus((const char *name, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareControlDBusTest, checkAuthentication)
{
    // Test bool getter: checkAuthentication()
    bool result = obj->checkAuthentication();
    EXPECT_FALSE(result);

}

TEST_F(ShareControlDBusTest, isValidUsername)
{
    // Test method: bool isValidUsername((const QString &username))
    QString _arg0{};
    auto result = obj->isValidUsername(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShareControlDBusTest, ShareControlDBus_Destructor)
{
    // Test method:  ~ShareControlDBus(())
    EXPECT_NO_FATAL_FAILURE({ ShareControlDBus *tmp = new ShareControlDBus(); delete tmp; });
}
