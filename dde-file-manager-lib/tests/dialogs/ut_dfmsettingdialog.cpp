#include "dialogs/dfmsettingdialog.h"
#include "interfaces/dfmapplication.h"

#include <gtest/gtest.h>


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

//TEST_F(TestDFMSettingDialog, testInit)
//{
//    m_pTesting->show();
//}
