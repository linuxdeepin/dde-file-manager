#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPushButton>
#include <DWaterProgress>

#define private public
#include "views/dfmvaultactivefinishedview.h"


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
    m_view->slotTimeout();
    EXPECT_FALSE(m_view->m_pWidget1->isVisible());
    EXPECT_FALSE(m_view->m_pWidget2->isVisible());
    EXPECT_TRUE(m_view->m_pFinishedBtn->isEnabled());
}
