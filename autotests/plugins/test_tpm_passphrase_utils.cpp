// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tpm_passphrase_utils.cpp
 * @brief Unit tests for tpm_passphrase_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class tpm_passphrase_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new tpm_passphrase_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    tpm_passphrase_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(tpm_passphrase_utilsTest, genPassphraseFromTPM)
{
    // Test method: int genPassphraseFromTPM((const QString &dev, const QString &pin, QString *passphrase))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->genPassphraseFromTPM(_arg0, _arg1, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(tpm_passphrase_utilsTest, getPassphraseFromTPM)
{
    // Test method: QString getPassphraseFromTPM((const QString &dev, const QString &pin))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->getPassphraseFromTPM(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
