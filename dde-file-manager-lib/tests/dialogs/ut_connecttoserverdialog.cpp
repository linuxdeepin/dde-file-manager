
#include <QComboBox>
#include <gtest/gtest.h>

#define private public
#include "dialogs/connecttoserverdialog.h"

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

TEST_F(TestConnectToServerDialog, testOnAddButtonClicked)
{
    m_pTester->m_serverComboBox->setEditText("123");
    m_pTester->onAddButtonClicked();
}

TEST_F(TestConnectToServerDialog, testOnDelButtonClicked)
{
    m_pTester->m_serverComboBox->setEditText("123");
    m_pTester->onDelButtonClicked();
}
