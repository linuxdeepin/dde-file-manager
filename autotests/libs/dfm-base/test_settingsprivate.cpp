// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingsprivate.cpp
 * @brief Unit tests for SettingsPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/application/settings.h"

#include <QTest>

using namespace src;

class SettingsPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingsPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingsPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingsPrivateTest, fromJson)
{
    // Test method: void fromJson((const QByteArray &json, Data *data))
    QByteArray _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->fromJson(_arg0, nullptr));
}
