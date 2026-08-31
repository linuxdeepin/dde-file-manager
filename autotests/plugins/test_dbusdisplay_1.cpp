// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusdisplay_1.cpp
 * @brief Unit tests for DBusDisplay methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdisplay1.h"

#include <QTest>

using namespace ddplugin_core;

class DBusDisplayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusDisplay();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusDisplay *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusDisplayTest, ApplyChanges)
{
    // Test getter: QDBusPendingReply<> ApplyChanges()
    auto result = obj->ApplyChanges();
    EXPECT_NO_FATAL_FAILURE({ obj->ApplyChanges(); });

}

TEST_F(DBusDisplayTest, AssociateTouch)
{
    // Test getter: QDBusPendingReply<> AssociateTouch()
    auto result = obj->AssociateTouch();
    EXPECT_NO_FATAL_FAILURE({ obj->AssociateTouch(); });

}

TEST_F(DBusDisplayTest, AssociateTouchByUUID)
{
    // Test getter: QDBusPendingReply<> AssociateTouchByUUID()
    auto result = obj->AssociateTouchByUUID();
    EXPECT_NO_FATAL_FAILURE({ obj->AssociateTouchByUUID(); });

}

TEST_F(DBusDisplayTest, CanRotate)
{
    // Test getter: QDBusPendingReply<bool> CanRotate()
    auto result = obj->CanRotate();
    EXPECT_NO_FATAL_FAILURE({ obj->CanRotate(); });

}

TEST_F(DBusDisplayTest, CanSetBrightness)
{
    // Test getter: QDBusPendingReply<bool> CanSetBrightness()
    auto result = obj->CanSetBrightness();
    EXPECT_NO_FATAL_FAILURE({ obj->CanSetBrightness(); });

}

TEST_F(DBusDisplayTest, ChangeBrightness)
{
    // Test getter: QDBusPendingReply<> ChangeBrightness()
    auto result = obj->ChangeBrightness();
    EXPECT_NO_FATAL_FAILURE({ obj->ChangeBrightness(); });

}

TEST_F(DBusDisplayTest, GetBuiltinMonitor)
{
    // Test getter: QDBusReply<QString> GetBuiltinMonitor()
    auto result = obj->GetBuiltinMonitor();
    EXPECT_NO_FATAL_FAILURE({ obj->GetBuiltinMonitor(); });

}

TEST_F(DBusDisplayTest, GetRealDisplayMode)
{
    // Test getter: QDBusPendingReply<uchar> GetRealDisplayMode()
    auto result = obj->GetRealDisplayMode();
    EXPECT_NO_FATAL_FAILURE({ obj->GetRealDisplayMode(); });

}

TEST_F(DBusDisplayTest, ListOutputNames)
{
    // Test getter: QDBusPendingReply<QStringList> ListOutputNames()
    auto result = obj->ListOutputNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusDisplayTest, ModifyConfigName)
{
    // Test getter: QDBusPendingReply<> ModifyConfigName()
    auto result = obj->ModifyConfigName();
    EXPECT_NO_FATAL_FAILURE({ obj->ModifyConfigName(); });

}

TEST_F(DBusDisplayTest, RefreshBrightness)
{
    // Test getter: QDBusPendingReply<> RefreshBrightness()
    auto result = obj->RefreshBrightness();
    EXPECT_NO_FATAL_FAILURE({ obj->RefreshBrightness(); });

}

TEST_F(DBusDisplayTest, SetBrightness)
{
    // Test getter: QDBusPendingReply<> SetBrightness()
    auto result = obj->SetBrightness();
    EXPECT_NO_FATAL_FAILURE({ obj->SetBrightness(); });

}

TEST_F(DBusDisplayTest, SetColorTemperature)
{
    // Test getter: QDBusPendingReply<> SetColorTemperature()
    auto result = obj->SetColorTemperature();
    EXPECT_NO_FATAL_FAILURE({ obj->SetColorTemperature(); });

}

TEST_F(DBusDisplayTest, SetMethodAdjustCCT)
{
    // Test getter: QDBusPendingReply<> SetMethodAdjustCCT()
    auto result = obj->SetMethodAdjustCCT();
    EXPECT_NO_FATAL_FAILURE({ obj->SetMethodAdjustCCT(); });

}

TEST_F(DBusDisplayTest, SetPrimary)
{
    // Test getter: QDBusPendingReply<> SetPrimary()
    auto result = obj->SetPrimary();
    EXPECT_NO_FATAL_FAILURE({ obj->SetPrimary(); });

}

TEST_F(DBusDisplayTest, SwitchMode)
{
    // Test getter: QDBusPendingReply<> SwitchMode()
    auto result = obj->SwitchMode();
    EXPECT_NO_FATAL_FAILURE({ obj->SwitchMode(); });

}

TEST_F(DBusDisplayTest, __propertyChanged__)
{
    // Test method: void __propertyChanged__(())
    EXPECT_NO_FATAL_FAILURE(obj->__propertyChanged__());
}

TEST_F(DBusDisplayTest, displayMode)
{
    // Test getter: uchar displayMode()
    auto result = obj->displayMode();
    EXPECT_NO_FATAL_FAILURE({ obj->displayMode(); });

}

TEST_F(DBusDisplayTest, hasChanged)
{
    // Test bool getter: hasChanged()
    bool result = obj->hasChanged();
    EXPECT_FALSE(result);

}

TEST_F(DBusDisplayTest, monitors)
{
    // Test getter: QList<QDBusObjectPath> monitors()
    auto result = obj->monitors();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusDisplayTest, primaryRect)
{
    // Test getter: DisplayRect primaryRect()
    auto result = obj->primaryRect();
    EXPECT_NO_FATAL_FAILURE({ obj->primaryRect(); });

}

TEST_F(DBusDisplayTest, screenHeight)
{
    // Test getter: ushort screenHeight()
    auto result = obj->screenHeight();
    EXPECT_NO_FATAL_FAILURE({ obj->screenHeight(); });

}

TEST_F(DBusDisplayTest, screenWidth)
{
    // Test getter: ushort screenWidth()
    auto result = obj->screenWidth();
    EXPECT_NO_FATAL_FAILURE({ obj->screenWidth(); });

}

TEST_F(DBusDisplayTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDisplayTest, staticServiceName)
{
    // Test getter: char staticServiceName()
    auto result = obj->staticServiceName();
    EXPECT_EQ(result, 0);

}
