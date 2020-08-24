#include "dialogs/closealldialogindicator.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

namespace  {
    class TestCloseAllDialogIndicator : public testing::Test{
    public:
        void SetUp() override
        {
            m_pTester = new CloseAllDialogIndicator();
            std::cout << "start TestCloseAllDialogIndicator";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestCloseAllDialogIndicator";
        }
    public:
        CloseAllDialogIndicator *m_pTester;
    };
}

TEST_F(TestCloseAllDialogIndicator, testInit)
{
//    m_pTester->show();
}

TEST_F(TestCloseAllDialogIndicator, testSetTotalMessage)
{
    qint64 size = 1024;
    int count = 2048;
    m_pTester->setTotalMessage(size, count);
}

TEST_F(TestCloseAllDialogIndicator, testKeyPressEvent_Escape)
{
    QTest::keyPress(m_pTester, Qt::Key_Escape);
}
