// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_comdeepinlicenseinterface.cpp
 * @brief Unit tests for ComDeepinLicenseInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/licenceInterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class ComDeepinLicenseInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComDeepinLicenseInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComDeepinLicenseInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComDeepinLicenseInterfaceTest, ComDeepinLicenseInterface)
{
    // Test constructor: ComDeepinLicenseInterface((const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComDeepinLicenseInterfaceTest, authorizationProperty)
{
    // Test getter: uint authorizationProperty()
    auto result = obj->authorizationProperty();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinLicenseInterfaceTest, authorizationState)
{
    // Test getter: int authorizationState()
    auto result = obj->authorizationState();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinLicenseInterfaceTest, serviceProperty)
{
    // Test getter: uint serviceProperty()
    auto result = obj->serviceProperty();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinLicenseInterfaceTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinLicenseInterfaceTest, ComDeepinLicenseInterface_Destructor)
{
    // Test method:  ~ComDeepinLicenseInterface(())
    EXPECT_NO_FATAL_FAILURE({ ComDeepinLicenseInterface *tmp = new ComDeepinLicenseInterface(); delete tmp; });
}
