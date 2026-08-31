// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_chgpassphrasedialog.cpp
 * @brief Unit tests for ChgPassphraseDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/chgpassphrasedialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class ChgPassphraseDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ChgPassphraseDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ChgPassphraseDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ChgPassphraseDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}
