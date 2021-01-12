#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <DPasswordEdit>
#include <QPushButton>
#include <DToolTip>

#define private public
#include "views/dfmvaultremovebypasswordview.h"


namespace  {
    class TestDFMVaultRemoveByPasswordView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRemoveByPasswordView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRemoveByPasswordView>(new DFMVaultRemoveByPasswordView());
            m_view->show();
            std::cout << "start TestDFMVaultRemoveByPasswordView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRemoveByPasswordView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRemoveByPasswordView, tst_getPassword)
{
    m_view->clear();

    EXPECT_EQ("", m_view->getPassword());

    m_view->m_pwdEdit->setText("123");

    EXPECT_EQ("123", m_view->getPassword());
}

TEST_F(TestDFMVaultRemoveByPasswordView, tst_show_tip)
{
    QTest::mouseClick(m_view->m_tipsBtn, Qt::LeftButton);
}

TEST_F(TestDFMVaultRemoveByPasswordView, tst_setTipsButtonVisible)
{
    m_view->setTipsButtonVisible(true);
   // EXPECT_TRUE(m_view->m_tooltip->isVisible());
}

