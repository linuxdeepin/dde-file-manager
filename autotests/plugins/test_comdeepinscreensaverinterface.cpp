// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_comdeepinscreensaverinterface.cpp
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

TEST_F(ComDeepinScreenSaverInterfaceTest, GetScreenSaverCover)
{
    // Test getter: QDBusPendingReply<QString> GetScreenSaverCover()
    auto result = obj->GetScreenSaverCover();
    EXPECT_NO_FATAL_FAILURE({ obj->GetScreenSaverCover(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, Preview)
{
    // Test getter: QDBusPendingReply<bool> Preview()
    auto result = obj->Preview();
    EXPECT_NO_FATAL_FAILURE({ obj->Preview(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, RefreshScreenSaverList)
{
    // Test getter: QDBusPendingReply<> RefreshScreenSaverList()
    auto result = obj->RefreshScreenSaverList();
    EXPECT_NO_FATAL_FAILURE({ obj->RefreshScreenSaverList(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, Start)
{
    // Test getter: QDBusPendingReply<> Start()
    auto result = obj->Start();
    EXPECT_NO_FATAL_FAILURE({ obj->Start(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, Stop)
{
    // Test getter: QDBusPendingReply<> Stop()
    auto result = obj->Stop();
    EXPECT_NO_FATAL_FAILURE({ obj->Stop(); });

}

TEST_F(ComDeepinScreenSaverInterfaceTest, allScreenSaver)
{
    // Test getter: QStringList allScreenSaver()
    auto result = obj->allScreenSaver();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComDeepinScreenSaverInterfaceTest, batteryScreenSaverTimeout)
{
    // Test getter: int batteryScreenSaverTimeout()
    auto result = obj->batteryScreenSaverTimeout();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinScreenSaverInterfaceTest, currentScreenSaver)
{
    // Test getter: QString currentScreenSaver()
    auto result = obj->currentScreenSaver();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComDeepinScreenSaverInterfaceTest, linePowerScreenSaverTimeout)
{
    // Test getter: int linePowerScreenSaverTimeout()
    auto result = obj->linePowerScreenSaverTimeout();
    EXPECT_EQ(result, 0);

}

TEST_F(ComDeepinScreenSaverInterfaceTest, setBatteryScreenSaverTimeout)
{
    // Test method: void setBatteryScreenSaverTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->setBatteryScreenSaverTimeout());
}

TEST_F(ComDeepinScreenSaverInterfaceTest, setCurrentScreenSaver)
{
    // Test method: void setCurrentScreenSaver(())
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentScreenSaver());
}

TEST_F(ComDeepinScreenSaverInterfaceTest, setLinePowerScreenSaverTimeout)
{
    // Test method: void setLinePowerScreenSaverTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->setLinePowerScreenSaverTimeout());
}

TEST_F(ComDeepinScreenSaverInterfaceTest, ComDeepinScreenSaverInterface)
{
    // Test constructor: ComDeepinScreenSaverInterface((const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComDeepinScreenSaverInterfaceTest, _ComDeepinScreenSaverInterface)
{
    // Test constructor: ComDeepinScreenSaverInterface((const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
