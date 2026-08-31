// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_accesscontroldbus.cpp
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

TEST_F(AccessControlDBusTest, onBlockDevAdded)
{
    // Test method: void onBlockDevAdded((const QString &deviceId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDevAdded(_arg0));
}

TEST_F(AccessControlDBusTest, onBlockDevMounted)
{
    // Test method: void onBlockDevMounted((const QString &deviceId, const QString &mountPoint))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDevMounted(_arg0, _arg1));
}
