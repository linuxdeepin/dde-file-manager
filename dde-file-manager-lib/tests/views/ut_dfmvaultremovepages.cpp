#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <DToolTip>

#define private public
#include "views/dfmvaultremovepages.h"

DWIDGET_USE_NAMESPACE
namespace  {
    class TestDFMVaultRemovePages: public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRemovePages> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRemovePages>(new DFMVaultRemovePages());
            m_view->show();
            std::cout << "start TestDFMVaultRemovePages" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRemovePages" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRemovePages, tst_getPassword)
{
    m_view->onButtonClicked(0);
    m_view->onButtonClicked(1);
    m_view->onButtonClicked(2);
    m_view->onButtonClicked(3);
}

TEST_F(TestDFMVaultRemovePages, tst_onLockVault)
{
    m_view->m_bRemoveVault = true;
    m_view->onLockVault(0);
}

TEST_F(TestDFMVaultRemovePages, tst_onVualtRemoveFinish)
{
    m_view->onVualtRemoveFinish(true);
    m_view->onVualtRemoveFinish(false);
    EXPECT_TRUE(m_view->getButton(0)->isEnabled());
}

