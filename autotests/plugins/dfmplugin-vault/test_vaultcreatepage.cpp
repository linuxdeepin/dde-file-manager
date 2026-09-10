// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include "stubext.h"

#define private public
#define protected public
#include "views/vaultcreatepage.h"
#undef protected
#undef private
#include "events/vaulteventcaller.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <dfm-base/utils/windowutils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class VaultActiveViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&WindowUtils::isWayLand, []() -> bool { return false; });
        stub.set_lamda(&OperatorCenter::createDirAndFile, []() -> Result { return { true, "" }; });
        stub.set_lamda(&VaultHelper::defaultCdAction, [](VaultHelper *, quint64, const QUrl &) {});
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        view = new VaultActiveView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultActiveView *view = nullptr;
};

TEST_F(VaultActiveViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultActiveViewTest, SlotNextWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotNextWidget());
}

TEST_F(VaultActiveViewTest, EncryptVault_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->encryptVault());
}

// --- setBeginingState ---

TEST_F(VaultActiveViewTest, SetBeginingState_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setBeginingState());
}

TEST_F(VaultActiveViewTest, Tr_ReturnsNonEmpty)
{
    EXPECT_FALSE(VaultActiveView::tr("test").isEmpty());
}
