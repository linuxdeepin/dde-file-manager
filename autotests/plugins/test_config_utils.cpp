// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_config_utils.cpp
 * @brief Unit tests for config_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class config_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new config_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    config_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(config_utilsTest, cipherType)
{
    // Test getter: QString cipherType()
    auto result = obj->cipherType();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(config_utilsTest, enableAlgoFromDConfig)
{
    // Test bool getter: enableAlgoFromDConfig()
    bool result = obj->enableAlgoFromDConfig();
    EXPECT_FALSE(result);

}

TEST_F(config_utilsTest, enableEncrypt)
{
    // Test bool getter: enableEncrypt()
    bool result = obj->enableEncrypt();
    EXPECT_FALSE(result);

}

TEST_F(config_utilsTest, exportKeyEnabled)
{
    // Test bool getter: exportKeyEnabled()
    bool result = obj->exportKeyEnabled();
    EXPECT_FALSE(result);

}

TEST_F(config_utilsTest, tpmAlgoFromDConfig)
{
    // Test method: bool tpmAlgoFromDConfig((QString *sessionHash, QString *sessionKey,
                                      QString *primaryHash, QString *primaryKey,
                                      QString *minorHash, QString *minorKey,
                                      QString *pcr, QString *pcrBank))
    auto result = obj->tpmAlgoFromDConfig(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(config_utilsTest, useOverlayDMMode)
{
    // Test bool getter: useOverlayDMMode()
    bool result = obj->useOverlayDMMode();
    EXPECT_FALSE(result);

}
