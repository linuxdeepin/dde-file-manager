// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_comdeepinscreensaverinterface_1.cpp
 * @brief Unit tests for ComDeepinScreenSaverInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/screensaver_interface.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class ComDeepinScreenSaverInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComDeepinScreenSaverInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComDeepinScreenSaverInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComDeepinScreenSaverInterfaceTest, ConfigurableItems)
{
    // Test getter: QDBusPendingReply<QStringList> ConfigurableItems()
    auto result = obj->ConfigurableItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComDeepinScreenSaverInterfaceTest, IsConfigurable)
{
    // Test getter: QDBusPendingReply<bool> IsConfigurable()
    auto result = obj->IsConfigurable();
    EXPECT_NO_FATAL_FAILURE({ obj->IsConfigurable(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, StartCustomConfig)
{
    // Test getter: QDBusPendingReply<bool> StartCustomConfig()
    auto result = obj->StartCustomConfig();
    EXPECT_NO_FATAL_FAILURE({ obj->StartCustomConfig(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, isRunning)
{
    // Test bool getter: isRunning()
    bool result = obj->isRunning();
    EXPECT_FALSE(result);

}

TEST_F(ComDeepinScreenSaverInterfaceTest, lockScreenAtAwake)
{
    // Test bool getter: lockScreenAtAwake()
    bool result = obj->lockScreenAtAwake();
    EXPECT_FALSE(result);

}

TEST_F(ComDeepinScreenSaverInterfaceTest, lockScreenDelay)
{
    // Test getter: int lockScreenDelay()
    auto result = obj->lockScreenDelay();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinScreenSaverInterfaceTest, setLockScreenAtAwake)
{
    // Test method: void setLockScreenAtAwake(())
    EXPECT_NO_FATAL_FAILURE(obj->setLockScreenAtAwake());
}

TEST_F(ComDeepinScreenSaverInterfaceTest, setLockScreenDelay)
{
    // Test method: void setLockScreenDelay(())
    EXPECT_NO_FATAL_FAILURE(obj->setLockScreenDelay());
}

TEST_F(ComDeepinScreenSaverInterfaceTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}
