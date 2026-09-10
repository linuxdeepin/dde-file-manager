// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QLabel>
#include <QScrollArea>
#include <QShowEvent>

#include "stubext.h"

#define private public
#define protected public
#include "views/vaultpropertyview/vaultpropertydialog.h"
#undef protected
#undef private

DPVAULT_USE_NAMESPACE

class VaultPropertyDialogTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        dialog = new VaultPropertyDialog();
    }

    void TearDown() override
    {
        stub.clear();
        delete dialog;
    }

    VaultPropertyDialog *dialog = nullptr;
};

TEST_F(VaultPropertyDialogTest, Constructor_CreatesDialog)
{
    EXPECT_NE(dialog, nullptr);
}

TEST_F(VaultPropertyDialogTest, SelectFileUrl_NoCrash)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test_vault");
    EXPECT_NO_FATAL_FAILURE(dialog->selectFileUrl(url));
}

