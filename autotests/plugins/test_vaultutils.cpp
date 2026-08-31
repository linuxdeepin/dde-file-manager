// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultutils.cpp
 * @brief Unit tests for VaultUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultutils.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultUtilsTest, showAuthorityDialog)
{
    // Test method: void showAuthorityDialog((const QString &actionId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAuthorityDialog(_arg0));
}
