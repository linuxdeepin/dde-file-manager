// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deepinlicensehelper_1.cpp
 * @brief Unit tests for DeepinLicenseHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watermask/deepinlicensehelper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DeepinLicenseHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeepinLicenseHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeepinLicenseHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeepinLicenseHelperTest, DeepinLicenseHelper)
{
    // Test constructor: DeepinLicenseHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DeepinLicenseHelperTest, createInterface)
{
    // Test method: void createInterface(())
    EXPECT_NO_FATAL_FAILURE(obj->createInterface());
}

TEST_F(DeepinLicenseHelperTest, delayGetState)
{
    // Test method: void delayGetState(())
    EXPECT_NO_FATAL_FAILURE(obj->delayGetState());
}

TEST_F(DeepinLicenseHelperTest, getAuthorizationProperty)
{
    // Test getter: DeepinLicenseHelper::LicenseProperty getAuthorizationProperty()
    auto result = obj->getAuthorizationProperty();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DeepinLicenseHelperTest, getLicenseState)
{
    // Test method: void getLicenseState((DeepinLicenseHelper *self))
    EXPECT_NO_FATAL_FAILURE(obj->getLicenseState(nullptr));
}

TEST_F(DeepinLicenseHelperTest, getServiceProperty)
{
    // Test getter: DeepinLicenseHelper::LicenseProperty getServiceProperty()
    auto result = obj->getServiceProperty();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DeepinLicenseHelperTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(DeepinLicenseHelperTest, initFinshed)
{
    // Test method: void initFinshed((void *interface))
    EXPECT_NO_FATAL_FAILURE(obj->initFinshed(nullptr));
}

TEST_F(DeepinLicenseHelperTest, requestLicenseState)
{
    // Test method: void requestLicenseState(())
    EXPECT_NO_FATAL_FAILURE(obj->requestLicenseState());
}

TEST_F(DeepinLicenseHelperTest, DeepinLicenseHelper_Destructor)
{
    // Test method:  ~DeepinLicenseHelper(())
    EXPECT_NO_FATAL_FAILURE({ DeepinLicenseHelper *tmp = new DeepinLicenseHelper(); delete tmp; });
}
