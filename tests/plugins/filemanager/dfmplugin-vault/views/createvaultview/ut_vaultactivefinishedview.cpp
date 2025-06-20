// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/createvaultview/vaultactivefinishedview.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"
#include "utils/policy/policymanager.h"

#include <gtest/gtest.h>

#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/event/eventdispatcher.h>

#include <DWaterProgress>

#include <QVariantMap>
#include <QShowEvent>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(UT_VaultActiveFinishedView, setFinishedBtnEnabled)
{
    VaultActiveFinishedView view;
    view.setFinishedBtnEnabled(true);
    EXPECT_TRUE(view.finishedBtn->isEnabled());
}

TEST(UT_VaultActiveFinishedView, slotEncryptComplete_Success)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::recordTime, [] { __DBG_STUB_INVOKE__ });
    typedef bool (EventDispatcherManager::*FuncType)(const QString &, const QString &, QString, QVariantMap &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [] { __DBG_STUB_INVOKE__ return true; });

    VaultActiveFinishedView view;
    view.slotEncryptComplete(0);
    EXPECT_TRUE(view.waterProgress->value() == 100);
}

TEST(UT_VaultActiveFinishedView, slotEncryptComplete_Fail)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showMessageDialog, [] { __DBG_STUB_INVOKE__ return 0; });

    VaultActiveFinishedView view;
    view.slotEncryptComplete(1);
    EXPECT_TRUE(view.waterProgress->value() != 100);
}

TEST(UT_VaultActiveFinishedView, slotEncryptVault_True)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultUtils::showAuthorityDialog, [] { __DBG_STUB_INVOKE__ });

    VaultActiveFinishedView view;
    view.finishedBtn->setText(VaultActiveFinishedView::tr("Encrypt"));
    view.slotEncryptVault();
    EXPECT_FALSE(view.finishedBtn->isEnabled());
}

TEST(UT_VaultActiveFinishedView, slotEncryptVault_False)
{
    bool isCdRoot { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::defaultCdAction, [ &isCdRoot ] { __DBG_STUB_INVOKE__ isCdRoot = true; });
    stub.set_lamda(&VaultHelper::recordTime, [] { __DBG_STUB_INVOKE__ });

    VaultActiveFinishedView view;
    view.finishedBtn->setText("");
    view.slotEncryptVault();
    EXPECT_TRUE(isCdRoot);
}

TEST(UT_VaultActiveFinishedView, slotTimeout)
{
    DDialog dialog;
    QWidget widget;
    widget.setParent(&dialog);
    VaultActiveFinishedView view;
    view.setParent(&widget);
    view.slotTimeout();
    EXPECT_TRUE(view.finishedBtn->isEnabled());
}

TEST(UT_VaultActiveFinishedView, showEvent)
{
    bool isSetValue { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::setVauleCurrentPageMark, [ &isSetValue ] { __DBG_STUB_INVOKE__ isSetValue = true; });

    VaultActiveFinishedView view;
    QShowEvent event;
    view.showEvent(&event);
    EXPECT_TRUE(isSetValue);
}
