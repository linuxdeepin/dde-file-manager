// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crypt_setup.cpp
 * @brief Unit tests for crypt_setup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/cryptsetup.h"

#include <QTest>

using namespace src;

class crypt_setupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new crypt_setup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    crypt_setup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(crypt_setupTest, csActivateDevice)
{
    // Test method: int csActivateDevice((const QString &dev, const QString &activateName, const QString &passphrase))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->csActivateDevice(_arg0, _arg1, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setupTest, csActivateDeviceByVolume)
{
    // Test method: int csActivateDeviceByVolume((const QString &dev, const QString &activateName, const QByteArray &volume))
    QString _arg0{};
    QString _arg1{};
    QByteArray _arg2{};
    auto result = obj->csActivateDeviceByVolume(_arg0, _arg1, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setupTest, csRemoveKeyslot)
{
    // Test method: int csRemoveKeyslot((const QString &dev, int keyslot))
    QString _arg0{};
    auto result = obj->csRemoveKeyslot(_arg0, 0);
    EXPECT_GE(result, 0);

}
