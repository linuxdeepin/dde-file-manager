// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tpmcontroldbus.cpp
 * @brief Unit tests for TPMControlDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/tpmcontrol/tpmcontroldbus.h"

#include <QTest>

using namespace src;

class TPMControlDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TPMControlDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TPMControlDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TPMControlDBusTest, Decrypt)
{
    // Test method: int Decrypt((const QDBusUnixFileDescriptor &params, QDBusUnixFileDescriptor &password))
    QDBusUnixFileDescriptor _arg0{};
    QDBusUnixFileDescriptor _arg1{};
    auto result = obj->Decrypt(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(TPMControlDBusTest, Encrypt)
{
    // Test method: int Encrypt((const QDBusUnixFileDescriptor &params))
    QDBusUnixFileDescriptor _arg0{};
    auto result = obj->Encrypt(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(TPMControlDBusTest, TPMControlDBus)
{
    // Test constructor: TPMControlDBus((const char *name, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
