#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPushButton>
#include <DWaterProgress>

#include "stub.h"
#include "vaultlockmanager.h"
#include <DDialog>
#include <DSecureString>
#include "controllers/vaultcontroller.h"
#include <DWaterProgress>

#define private public
#include "views/dfmvaultactivefinishedview.h"

using namespace PolkitQt1;

namespace  {
    class TestDFMVaultActiveFinishedView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveFinishedView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveFinishedView>(new DFMVaultActiveFinishedView());

            std::cout << "start TestDFMVaultActiveFinishedView" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMVaultActiveFinishedView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveFinishedView, tst_setFinishedBtnEnabled)
{
    m_view->setFinishedBtnEnabled(true);
    EXPECT_TRUE(m_view->m_pFinishedBtn->isEnabled());

    m_view->setFinishedBtnEnabled(false);
    EXPECT_FALSE(m_view->m_pFinishedBtn->isEnabled());

    EXPECT_FALSE(m_view->m_pWidget2->isVisible());
    EXPECT_FALSE(m_view->m_pWidget3->isVisible());
}

TEST_F(TestDFMVaultActiveFinishedView, tst_slotEncryptComplete)
{
    m_view->slotEncryptComplete(0);
    EXPECT_EQ(100, m_view->m_pWaterProgress->value());
}

TEST_F(TestDFMVaultActiveFinishedView, tst_slotTimeout)
{
    DDialog *parent = new DDialog();
    m_view->setParent(parent);
    parent->deleteLater();

    m_view->slotTimeout();
    EXPECT_FALSE(m_view->m_pWidget1->isVisible());
    EXPECT_FALSE(m_view->m_pWidget2->isVisible());
    EXPECT_TRUE(m_view->m_pFinishedBtn->isEnabled());
}

TEST_F(TestDFMVaultActiveFinishedView, tst_slotEncryptVault)
{
    void(*st_checkAuthentication)(const QString&, const Subject&, Authority::AuthorizationFlags) = [](const QString&, const Subject&, Authority::AuthorizationFlags){};
    Stub stub;
    stub.set(ADDR(Authority, checkAuthorization), st_checkAuthentication);

    DDialog *parent = new DDialog();
    m_view->setParent(parent);
    parent->deleteLater();

    void (*st_createVault)(const Dtk::DCORE_NAMESPACE::DSecureString &, QString, QString) =
            [](const Dtk::DCORE_NAMESPACE::DSecureString &, QString, QString){
        // do nothing.
    };
    stub.set(ADDR(VaultController, createVault), st_createVault);

    void (*st_start)() = [](){
        // do nothing.
    };
    stub.set(ADDR(DWaterProgress, start), st_start);

    m_view->slotEncryptVault();
}
