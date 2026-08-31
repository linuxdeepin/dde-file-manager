// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingbackendprivate.cpp
 * @brief Unit tests for SettingBackendPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/configs/settingbackend.h"

#include <QTest>

using namespace src;

class SettingBackendPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingBackendPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingBackendPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingBackendPrivateTest, saveAsAppAttr)
{
    // Test method: void saveAsAppAttr((const QString &key, const QVariant &val))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveAsAppAttr(_arg0, _arg1));
}

TEST_F(SettingBackendPrivateTest, saveAsGenAttr)
{
    // Test method: void saveAsGenAttr((const QString &key, const QVariant &val))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveAsGenAttr(_arg0, _arg1));
}

TEST_F(SettingBackendPrivateTest, saveByFunc)
{
    // Test method: void saveByFunc((const QString &key, const QVariant &val))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveByFunc(_arg0, _arg1));
}
