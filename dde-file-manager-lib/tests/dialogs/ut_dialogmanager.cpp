#include "dialogs/dialogmanager.h"
#include "dfmevent.h"

#include <gtest/gtest.h>

namespace  {
    class TestDialogManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DialogManager();
            std::cout << "start TestDialogManager";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDialogManager";
        }
    public:
        DialogManager *m_pTester;
    };
}

//! 测试文件命名为“..”时，弹出提示框
TEST_F(TestDialogManager, showDotDotErrorDialog)
{
    DFMEvent event;
    int result = m_pTester->showRenameNameDotDotErrorDialog(event);
    EXPECT_TRUE(result == 0 || result == 1);
}
