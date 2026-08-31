// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dialog_utils_1.cpp
 * @brief Unit tests for dialog_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class dialog_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new dialog_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    dialog_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(dialog_utilsTest, showConfirmDecryptionDialog)
{
    // Test method: int showConfirmDecryptionDialog((const QString &device, bool needReboot))
    QString _arg0{};
    auto result = obj->showConfirmDecryptionDialog(_arg0, false);
    EXPECT_GE(result, 0);

}

TEST_F(dialog_utilsTest, showConfirmEncryptionDialog)
{
    // Test method: int showConfirmEncryptionDialog((const QString &device, bool needReboot))
    QString _arg0{};
    auto result = obj->showConfirmEncryptionDialog(_arg0, false);
    EXPECT_GE(result, 0);

}
