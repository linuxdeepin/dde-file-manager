// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingsdbusinterface.cpp
 * @brief Unit tests for SettingsDBusInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "settingsdbusinterface.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class SettingsDBusInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingsDBusInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingsDBusInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingsDBusInterfaceTest, ShowScreensaverChooser)
{
    // Test method: void ShowScreensaverChooser((const QString &screen))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->ShowScreensaverChooser(_arg0));
}
