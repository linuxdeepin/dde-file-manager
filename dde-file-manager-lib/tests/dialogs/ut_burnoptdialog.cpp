#include "dialogs/burnoptdialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestBurnOptDialog : public testing::Test{
    public:
        void SetUp() override
        {
            m_pTester = new BurnOptDialog("/media/jerry3");
            std::cout << "start TestBurnOptDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBurnOptDialog";
        }
    public:
        BurnOptDialog *m_pTester;
    };
}

TEST_F(TestBurnOptDialog, testInit)
{
//    m_pTester->show();
}
