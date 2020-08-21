#include "dialogs/usersharepasswordsettingdialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestUserSharePasswordSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new UserSharePasswordSettingDialog();
            std::cout << "start TestTrashPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestTrashPropertyDialog";
        }
    public:
        UserSharePasswordSettingDialog  *m_pTester;
    };
}

TEST_F(TestUserSharePasswordSettingDialog, testInit)
{
    m_pTester->show();
}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked)
{
    m_pTester->onButtonClicked(0);
}
