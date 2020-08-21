#include "dialogs/basedialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestBaseDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new BaseDialog();
            std::cout << "start TestBaseDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBaseDialog";
        }
    public:
        BaseDialog *m_pTester;
    };
}

TEST_F(TestBaseDialog, testInit)
{
//    m_pTester->show();
}

TEST_F(TestBaseDialog, testSetTitle)
{
    QString strTitle("TitleName");
    m_pTester->setTitle(strTitle);
}
