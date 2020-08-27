#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <DToolTip>

#define private public
#include "views/dfmvaultremovebyrecoverykeyview.h"


namespace  {
    class TestDFMVaultRemoveByRecoverykeyView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRemoveByRecoverykeyView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRemoveByRecoverykeyView>(new DFMVaultRemoveByRecoverykeyView());
            m_view->show();
            std::cout << "start TestDFMVaultRemoveByRecoverykeyView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRemoveByRecoverykeyView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRemoveByRecoverykeyView, tst_getPassword)
{
    m_view->clear();

    EXPECT_EQ("", m_view->getRecoverykey());

    m_view->m_keyEdit->setPlainText("123");

    EXPECT_EQ("123", m_view->getRecoverykey());

    EXPECT_NO_FATAL_FAILURE(QTest::keyClicks(m_view->m_keyEdit, "123465678aA!"));
    EXPECT_NO_FATAL_FAILURE(QTest::keyClicks(m_view->m_keyEdit,
                     "12346567891011121314151611234656789101112131415161aA!"));
}

TEST_F(TestDFMVaultRemoveByRecoverykeyView, tst_eventFilter)
{
    QTest::keyClick(m_view->m_keyEdit, Qt::Key_Enter);
}

