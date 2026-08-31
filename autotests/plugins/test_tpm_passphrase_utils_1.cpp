// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tpm_passphrase_utils_1.cpp
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

TEST_F(tpm_passphrase_utilsTest, genPassphraseFromTPM_NonBlock)
{
    // Test method: int genPassphraseFromTPM_NonBlock((const QString &dev, const QString &pin, QString *passphrase))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->genPassphraseFromTPM_NonBlock(_arg0, _arg1, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(tpm_passphrase_utilsTest, getAlgorithm)
{
    // Test method: bool getAlgorithm((QString *sessionHashAlgo, QString *sessionKeyAlgo,
                                        QString *primaryHashAlgo, QString *primaryKeyAlgo,
                                        QString *minorHashAlgo, QString *minorKeyAlgo,
                                        QString *pcr, QString *pcrbank))
    auto result = obj->getAlgorithm(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(tpm_passphrase_utilsTest, getGlobalTPMConfigPath)
{
    // Test getter: QString getGlobalTPMConfigPath()
    auto result = obj->getGlobalTPMConfigPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(tpm_passphrase_utilsTest, getPassphraseFromTPM_NonBlock)
{
    // Test method: QString getPassphraseFromTPM_NonBlock((const QString &dev, const QString &pin))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->getPassphraseFromTPM_NonBlock(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(tpm_passphrase_utilsTest, tpmSupportInterAlgo)
{
    // Test bool getter: tpmSupportInterAlgo()
    bool result = obj->tpmSupportInterAlgo();
    EXPECT_FALSE(result);

}

TEST_F(tpm_passphrase_utilsTest, tpmSupportSMAlgo)
{
    // Test bool getter: tpmSupportSMAlgo()
    bool result = obj->tpmSupportSMAlgo();
    EXPECT_FALSE(result);

}
