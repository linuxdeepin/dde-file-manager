#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPushButton>
#include <QPlainTextEdit>
#include <QTest>
#include <DPasswordEdit>
#include <QTimer>

#define private public
#include "views/dfmvaultactivesetunlockmethodview.h"


namespace  {
    class TestDFMVaultActiveSetUnlockMethodView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveSetUnlockMethodView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveSetUnlockMethodView>(new DFMVaultActiveSetUnlockMethodView());
            m_view->show();
            std::cout << "start TestDFMVaultActiveSetUnlockMethodView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveSetUnlockMethodView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveSetUnlockMethodView, tst_passwordEditing)
{
    m_view->clearText();
    m_view->m_pPassword->setText("123");
    m_view->slotPasswordEditing();
    EXPECT_FALSE(m_view->checkInputInfo());

    m_view->m_pRepeatPassword->setText("123");
    m_view->slotRepeatPasswordEditing();
    EXPECT_FALSE(m_view->checkInputInfo());

    m_view->m_pPassword->setText("12345678aA!");
    m_view->slotPasswordEditing();
    m_view->m_pRepeatPassword->setText("12345678aA!");
    m_view->slotRepeatPasswordEditing();

    EXPECT_TRUE(m_view->m_pNext->isEnabled());
}

TEST_F(TestDFMVaultActiveSetUnlockMethodView, tst_passwordEditFinished)
{
    m_view->clearText();
    m_view->m_pPassword->setText("123");
    m_view->slotPasswordEditFinished();

    EXPECT_FALSE(m_view->m_pNext->isEnabled());

    m_view->m_pPassword->setText("12345678aA!");
    m_view->m_pRepeatPassword->setText("12345678aA!");
    m_view->slotPasswordEditFinished();
    m_view->slotRepeatPasswordEditFinished();

    EXPECT_TRUE(m_view->m_pNext->isEnabled());
}

TEST_F(TestDFMVaultActiveSetUnlockMethodView, tst_editFocusChanged)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotPasswordEditFocusChanged(true));
    EXPECT_NO_FATAL_FAILURE(m_view->slotRepeatPasswordEditFocusChanged(true));
}

TEST_F(TestDFMVaultActiveSetUnlockMethodView, tst_slotGenerateEditChanged)
{
    m_view->slotGenerateEditChanged("12345678aA!");
    EXPECT_TRUE(m_view->m_pNext->isEnabled());

    m_view->slotGenerateEditChanged("123");
    EXPECT_FALSE(m_view->m_pNext->isEnabled());
}

TEST_F(TestDFMVaultActiveSetUnlockMethodView, tst_typeChanged)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotTypeChanged(1));
    EXPECT_NO_FATAL_FAILURE(m_view->slotTypeChanged(0));
}
