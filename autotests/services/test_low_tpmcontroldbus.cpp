// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_tpmcontroldbus.cpp
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

TEST_F(TPMControlDBusTest, CheckTPMLockout)
{
    // Test getter: int CheckTPMLockout()
    auto result = obj->CheckTPMLockout();
    EXPECT_EQ(result, 0);

}

TEST_F(TPMControlDBusTest, GetRandom)
{
    // Test method: int GetRandom((int size, QDBusUnixFileDescriptor &randomData))
    QDBusUnixFileDescriptor _arg1{};
    auto result = obj->GetRandom(0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(TPMControlDBusTest, IsSupportAlgo)
{
    // Test method: int IsSupportAlgo((const QString &algoName, bool &support))
    QString _arg0{};
    bool _arg1{};
    auto result = obj->IsSupportAlgo(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(TPMControlDBusTest, IsTPMAvailable)
{
    // Test getter: int IsTPMAvailable()
    auto result = obj->IsTPMAvailable();
    EXPECT_EQ(result, 0);

}

TEST_F(TPMControlDBusTest, OwnerAuthStatus)
{
    // Test getter: int OwnerAuthStatus()
    auto result = obj->OwnerAuthStatus();
    EXPECT_EQ(result, 0);

}

TEST_F(TPMControlDBusTest, checkAuthentication)
{
    // Test method: bool checkAuthentication((const QString &actionId))
    QString _arg0{};
    auto result = obj->checkAuthentication(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TPMControlDBusTest, sendDataViaFd)
{
    // Test method: bool sendDataViaFd((const QByteArray &data, QDBusUnixFileDescriptor &fd))
    QByteArray _arg0{};
    QDBusUnixFileDescriptor _arg1{};
    auto result = obj->sendDataViaFd(_arg0, _arg1);
    EXPECT_FALSE(result);

}
