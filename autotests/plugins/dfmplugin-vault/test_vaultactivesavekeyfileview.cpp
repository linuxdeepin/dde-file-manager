// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QShowEvent>

#include "stubext.h"

#define private public
#define protected public
#include "views/createvaultview/vaultactivesavekeyfileview.h"
#undef protected
#undef private
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/encryption/interfaceactivevault.h"

DPVAULT_USE_NAMESPACE

class VaultActiveSaveKeyFileViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        view = new VaultActiveSaveKeyFileView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultActiveSaveKeyFileView *view = nullptr;
};

TEST_F(VaultActiveSaveKeyFileViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultActiveSaveKeyFileViewTest, SetEncryptInfo_NoCrash)
{
    EncryptInfo info;
    info.mode = EncryptMode::kKeyMode;
    EXPECT_NO_FATAL_FAILURE(view->setEncryptInfo(info));
}

TEST_F(VaultActiveSaveKeyFileViewTest, SetNextButtonText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setNextButtonText("Next"));
}

TEST_F(VaultActiveSaveKeyFileViewTest, SetOldPasswordSchemeMigrationMode_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setOldPasswordSchemeMigrationMode(true));
    EXPECT_NO_FATAL_FAILURE(view->setOldPasswordSchemeMigrationMode(false));
}

TEST_F(VaultActiveSaveKeyFileViewTest, SlotChangeEdit_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotChangeEdit("/tmp"));
}

TEST_F(VaultActiveSaveKeyFileViewTest, SlotSelectCurrentFile_File_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotSelectCurrentFile("/tmp/testkey"));
}

TEST_F(VaultActiveSaveKeyFileViewTest, SlotSelectCurrentFile_Dir_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotSelectCurrentFile("/tmp"));
}

// --- showEvent ---

TEST_F(VaultActiveSaveKeyFileViewTest, ShowEvent_NoCrash)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(view->showEvent(&event));
}
