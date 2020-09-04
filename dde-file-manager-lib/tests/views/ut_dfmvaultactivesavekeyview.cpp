#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPushButton>
#include <DWaterProgress>
#include <DLabel>
#include <QPlainTextEdit>

#define private public
#include "views/dfmvaultactivesavekeyview.h"


namespace  {
    class TestDFMVaultActiveSaveKeyView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveSaveKeyView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveSaveKeyView>(new DFMVaultActiveSaveKeyView());
            m_view->show();
            std::cout << "start TestDFMVaultActiveSaveKeyView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveSaveKeyView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveSaveKeyView, tst_showEvent)
{
    EXPECT_FALSE(m_view->isHidden());
    m_view->hide();
    EXPECT_TRUE(m_view->isHidden());
}

TEST_F(TestDFMVaultActiveSaveKeyView, tst_slotKeyBtnClicked)
{
    m_view->slotKeyBtnClicked();
    EXPECT_FALSE(m_view->m_pQRCodeImage->isVisible());
    EXPECT_TRUE(m_view->m_pKeyText->isVisible());
}

TEST_F(TestDFMVaultActiveSaveKeyView, tst_slotQRCodeBtnClicked)
{
    m_view->slotQRCodeBtnClicked();
    EXPECT_FALSE(m_view->m_pKeyText->isVisible());
    EXPECT_TRUE(m_view->m_pQRCodeImage->isVisible());
    EXPECT_TRUE(m_view->m_pScanTipsLabel->isVisible());
}
