#include <gtest/gtest.h>
#include <DTitlebar>

#include "stub.h"

#include "dfmglobal.h"
#define private public
#include "dialogs/basedialog.h"

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
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);
    BaseDialog dlg;
    EXPECT_NE(nullptr, m_pTester->m_titlebar);
}

TEST_F(TestBaseDialog, testSetTitle)
{
    QString strTitle("TitleName");
    m_pTester->setTitle(strTitle);
    QString str = m_pTester->m_titlebar->windowTitle();
    EXPECT_TRUE(str.isEmpty());
}
