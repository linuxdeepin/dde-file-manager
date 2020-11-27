#include "dialogs/usersharepasswordsettingdialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestUserSharePasswordSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new UserSharePasswordSettingDialog();
            std::cout << "start TestUserSharePasswordSettingDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestUserSharePasswordSettingDialog";
        }
    public:
        UserSharePasswordSettingDialog  *m_pTester;
    };
}

TEST_F(TestUserSharePasswordSettingDialog, testInit)
{

}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked)
{
    m_pTester->onButtonClicked(1);
}
