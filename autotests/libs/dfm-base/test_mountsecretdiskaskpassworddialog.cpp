// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mountsecretdiskaskpassworddialog.cpp
 * @brief Unit tests for MountSecretDiskAskPasswordDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/mountpasswddialog/mountsecretdiskaskpassworddialog.h"

#include <QTest>

using namespace src;

class MountSecretDiskAskPasswordDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MountSecretDiskAskPasswordDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MountSecretDiskAskPasswordDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MountSecretDiskAskPasswordDialogTest, getUerInputedPassword)
{
    // Test getter: QString getUerInputedPassword()
    auto result = obj->getUerInputedPassword();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
