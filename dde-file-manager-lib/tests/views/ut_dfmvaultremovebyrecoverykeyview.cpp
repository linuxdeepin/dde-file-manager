#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <DToolTip>
#include <DDialog>
#include "dfmevent.h"

#include "stub.h"

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
    QSharedPointer<QKeyEvent> event = dMakeEventPointer<QKeyEvent>(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);

    EXPECT_FALSE(m_view->eventFilter(nullptr, event.get()));
    EXPECT_TRUE(m_view->eventFilter(m_view->m_keyEdit, event.get()));
}

TEST_F(TestDFMVaultRemoveByRecoverykeyView, tst_showAlertMessage)
{
    DTK_WIDGET_NAMESPACE::DDialog *dialog = new DTK_WIDGET_NAMESPACE::DDialog();
    m_view->setParent(dialog);
    m_view->showAlertMessage("");
    dialog->deleteLater();
}

TEST_F(TestDFMVaultRemoveByRecoverykeyView, tst_onRecoveryKeyChanged)
{
    m_view->onRecoveryKeyChanged();
    int pos = m_view->m_keyEdit->textCursor().position();
    EXPECT_EQ(static_cast<int>(0), pos);

    QString (*st_toPlainText)() = []()->QString {
        return "12346567891111213141516112346567891113333141516112aA!";
    };

    Stub stub;
    stub.set(ADDR(QPlainTextEdit, toPlainText), st_toPlainText);

    m_view->onRecoveryKeyChanged();
    int pos2 = m_view->m_keyEdit->textCursor().position();
    EXPECT_EQ(pos2, 38);
}
