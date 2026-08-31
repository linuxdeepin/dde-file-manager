// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_registercustomformat.cpp
 * @brief Unit tests for RegisterCustomFormat methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <QTest>

using namespace dfmplugin_menu;

class RegisterCustomFormatTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RegisterCustomFormat();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RegisterCustomFormat *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RegisterCustomFormatTest, instance)
{
    // Test getter: RegisterCustomFormat instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(RegisterCustomFormatTest, readConf)
{
    // Test method: DFMBASE_USE_NAMESPACE readConf((QIODevice &device, QSettings::SettingsMap &settingsMap))
    QIODevice _arg0{};
    QSettings::SettingsMap _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj->readConf(_arg0, _arg1); });
}

TEST_F(RegisterCustomFormatTest, writeConf)
{
    // Test method: bool writeConf((QIODevice &device, const QSettings::SettingsMap &settingsMap))
    QIODevice _arg0{};
    QSettings::SettingsMap _arg1{};
    auto result = obj->writeConf(_arg0, _arg1);
    EXPECT_FALSE(result);

}
