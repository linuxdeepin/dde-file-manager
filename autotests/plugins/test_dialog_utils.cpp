// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dialog_utils.cpp
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

TEST_F(dialog_utilsTest, isWayland)
{
    // Test bool getter: isWayland()
    bool result = obj->isWayland();
    EXPECT_FALSE(result);

}

TEST_F(dialog_utilsTest, showDialog)
{
    // Test method: int showDialog((const QString &title, const QString &msg))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->showDialog(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(dialog_utilsTest, showTPMError)
{
    // Test method: void showTPMError((const QString &title, tpm_passphrase_utils::TPMError err))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showTPMError(_arg0, {}));
}
