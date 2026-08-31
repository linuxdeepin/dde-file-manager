// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_accesscontroldbus.cpp
 * @brief Unit tests for AccessControlDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/accesscontrol/accesscontroldbus.h"

#include <QTest>

using namespace src;

class AccessControlDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AccessControlDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AccessControlDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AccessControlDBusTest, AccessControlDBus)
{
    // Test constructor: AccessControlDBus((const char *name, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AccessControlDBusTest, Chmod)
{
    // Test method: bool Chmod((const QString &path, uint mode))
    QString _arg0{};
    auto result = obj->Chmod(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(AccessControlDBusTest, changeMountedOnInit)
{
    // Test method: void changeMountedOnInit(())
    EXPECT_NO_FATAL_FAILURE(obj->changeMountedOnInit());
}

TEST_F(AccessControlDBusTest, checkAuthentication)
{
    // Test method: bool checkAuthentication((const QString &id))
    QString _arg0{};
    auto result = obj->checkAuthentication(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AccessControlDBusTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(AccessControlDBusTest, AccessControlDBus_Destructor)
{
    // Test method:  ~AccessControlDBus(())
    EXPECT_NO_FATAL_FAILURE({ AccessControlDBus *tmp = new AccessControlDBus(); delete tmp; });
}
