#include "dialogs/dfmsettingdialog.h"

#include <gtest/gtest.h>
#include <interfaces/dfmapplication.h>

namespace  {
    class TestDFMSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start DFMSettingDialog";
        }
        void TearDown() override
        {
            std::cout << "end DFMSettingDialog";
        }
    public:
        DFMSettingDialog *m_pTesting;
    };
}
