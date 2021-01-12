#include "stub.h"
#include "dfmglobal.h"
#include "usershare/usersharemanager.h"

#include <gtest/gtest.h>

#define private public
#include "dialogs/usersharepasswordsettingdialog.h"

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
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->onButtonClicked(1));
}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked2)
{
    DPasswordEdit edit;
    edit.setText("123");
    m_pTester->m_passwordEdit = &edit;

    void(*stub_setSambaPassword)(const QString&, const QString&) = [](const QString&, const QString&){

    };
    Stub stu;
    stu.set(ADDR(UserShareManager, setSambaPassword), stub_setSambaPassword);

    EXPECT_NO_FATAL_FAILURE(m_pTester->onButtonClicked(1));
}

TEST_F(TestUserSharePasswordSettingDialog, testInitUI)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    m_pTester->initUI();
    int width = m_pTester->width();
    EXPECT_EQ(width, 390);
}
