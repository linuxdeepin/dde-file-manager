#include "dialogs/connecttoserverdialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestConnectToServerDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new ConnectToServerDialog();
            std::cout << "start TestConnectToServerDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestConnectToServerDialog";
        }
    public:
        ConnectToServerDialog *m_pTester;
    };
}

TEST_F(TestConnectToServerDialog, testInit)
{
    m_pTester->show();
}

TEST_F(TestConnectToServerDialog, testOnButtonClicked)
{
    int index = 1;
    m_pTester->onButtonClicked(index);
}
