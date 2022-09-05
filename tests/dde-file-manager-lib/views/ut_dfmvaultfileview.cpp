// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public
#include "views/dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultremovepages.h"
#include "stub.h"
#include "dfmevent.h"


namespace  {
class TestDFMVaultFileView : public testing::Test
{
public:
    DFMVaultFileView* m_view;

    virtual void SetUp() override
    {
        m_view = new DFMVaultFileView();
        m_view->show();
        std::cout << "start TestDFMVaultFileView" << std::endl;
    }

    virtual void TearDown() override
    {
        m_view->close();
        delete m_view;
        m_view = nullptr;
        std::cout << "end TestDFMVaultFileView" << std::endl;
    }
};
}


TEST_F(TestDFMVaultFileView, tst_setRootUrl)
{
    // 对exec函数打桩
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&Dtk::Widget::DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    // 对DFMVaultPageBase::showTop()函数打桩
    void(*stub_showTop1)() = [](){};
    Stub stub2;
    typedef void(*fptr1)();
    fptr1 pShowTop1 = (fptr1)(&DFMVaultPageBase::showTop);
    stub2.set(pShowTop1, stub_showTop1);

    // 对DFMVaultRemovePages::showTop()
    void(*stub_showTop2)() = [](){};
    Stub stub3;
    fptr1 pShowTop2 = (fptr1)(&DFMVaultRemovePages::showTop);
    stub3.set(pShowTop2, stub_showTop2);

    DUrl url = VaultController::makeVaultUrl();
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    url.setHost("delete");
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    url.setHost("certificate");
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    // replace VaultController::state
    VaultController::VaultState(*st_state)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::NotAvailable;
    };
    Stub stub;
    stub.set(ADDR(VaultController, state), st_state);
    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    stub.reset(ADDR(VaultController, state));
    url.setHost("delete");

    // replace VaultController::state
    VaultController::VaultState(*st_state_unLocked)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::Unlocked;
    };
    stub.set(ADDR(VaultController, state), st_state_unLocked);

    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));

    // replace VaultController::state
    VaultController::VaultState(*st_state_encrypted)(QString lockBaseDir) =
    [](QString lockBaseDir)->VaultController::VaultState {
        Q_UNUSED(lockBaseDir)
        return VaultController::Encrypted;
    };
    stub.set(ADDR(VaultController, state), st_state_encrypted);

    EXPECT_NO_FATAL_FAILURE(m_view->setRootUrl(url));
}

TEST_F(TestDFMVaultFileView, tst_onLeaveVault)
{
    EXPECT_NO_FATAL_FAILURE(m_view->onLeaveVault(0));
}

TEST_F(TestDFMVaultFileView, tst_eventFilter)
{
    QSharedPointer<QMouseEvent> event = dMakeEventPointer<QMouseEvent>(
                                            QMouseEvent::KeyPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    m_view->eventFilter(nullptr, event.get());
}
