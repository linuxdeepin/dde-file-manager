#include "dialogs/dfmsettingdialog.h"
#include "interfaces/dfmapplication.h"

#include <gtest/gtest.h>
#include <DSettingsOption>


namespace  {
    class TestDFMSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTesting = new DFMSettingDialog();
            std::cout << "start TestDFMSettingDialog";
        }
        void TearDown() override
        {
            delete  m_pTesting;
            m_pTesting = nullptr;
            std::cout << "end TestDFMSettingDialog";
        }
    public:
        DFMSettingDialog *m_pTesting;
    };
}

TEST_F(TestDFMSettingDialog, testInit)
{

}

TEST_F(TestDFMSettingDialog, testCreateAutoMountCheckBox)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountCheckBox(&opt);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountOpenCheckBox)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountOpenCheckBox(&opt);
}
