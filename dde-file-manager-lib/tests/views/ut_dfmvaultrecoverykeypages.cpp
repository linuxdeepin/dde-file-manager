#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "dfmevent.h"

#include "stub.h"
#include "../stub-ext/stubext.h"

#include <QWindow>
#include <QTest>
#include <QPlainTextEdit>
#include <QPushButton>

#define private public
#include "views/dfmvaultrecoverykeypages.h"

namespace  {
    class TestDFMVaultRecoveryKeyPages : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultRecoveryKeyPages> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultRecoveryKeyPages>(new DFMVaultRecoveryKeyPages());
            m_view->show();
            std::cout << "start TestDFMVaultRecoveryKeyPages" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRecoveryKeyPages" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultRecoveryKeyPages, tst_buttonClicked)
{
    m_view->onButtonClicked(1);
    m_view->onButtonClicked(0);
}

TEST_F(TestDFMVaultRecoveryKeyPages, tst_recoveryKeyChanged)
{
    QTest::keyClicks(m_view->m_recoveryKeyEdit, "");
    EXPECT_FALSE(m_view->getButton(1)->isEnabled());

    EXPECT_NO_FATAL_FAILURE(QTest::keyClicks(m_view->m_recoveryKeyEdit, "123465678aA!"));
    EXPECT_NO_FATAL_FAILURE(QTest::keyClicks(m_view->m_recoveryKeyEdit,
                     "12346567891011121314151611234656789101112131415161aA!"));

    QString (*st_toPlainText)() = []()->QString {
        return "0123456789012345678901234567890123456789";
    };
    Stub stub;
    stub.set(ADDR(QPlainTextEdit, toPlainText), st_toPlainText);
    m_view->recoveryKeyChanged();
}

TEST_F(TestDFMVaultRecoveryKeyPages, tst_onUnlockVault)
{
    int (*st_exec)() = []()->int{
        // do nothing.
        return DDialog::Accepted;
    };
    stub_ext::StubExt stub;
    stub.set(VADDR(DDialog, exec), st_exec);

    m_view->m_bUnlockByKey = true;
    m_view->onUnlockVault(1);
    EXPECT_FALSE(m_view->isHidden());
}

TEST_F(TestDFMVaultRecoveryKeyPages, tst_eventFilter)
{
    QSharedPointer<QKeyEvent> event = dMakeEventPointer<QKeyEvent>(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);

    EXPECT_FALSE(m_view->eventFilter(nullptr, event.get()));
    EXPECT_TRUE(m_view->eventFilter(m_view->m_recoveryKeyEdit, event.get()));
}

TEST_F(TestDFMVaultRecoveryKeyPages, tst_afterRecoveryKeyChanged)
{
    QString key("");
    m_view->afterRecoveryKeyChanged(key);
    key = "key";
    m_view->afterRecoveryKeyChanged(key);
}

TEST_F(TestDFMVaultRecoveryKeyPages, tst_showAlertMessage)
{
    DTK_WIDGET_NAMESPACE::DDialog *dialog = new DTK_WIDGET_NAMESPACE::DDialog();
    m_view->setParent(dialog);
    m_view->showAlertMessage("");
    dialog->deleteLater();
}


