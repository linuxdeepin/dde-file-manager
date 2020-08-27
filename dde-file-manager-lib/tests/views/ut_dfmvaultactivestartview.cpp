#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "views/dfmvaultactivestartview.h"


namespace  {
    class TestDFMVaultActiveStartView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveStartView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveStartView>(new DFMVaultActiveStartView());
            m_view->show();
            std::cout << "start TestDFMVaultActiveStartView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveStartView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveStartView, tst_slotStartBtnClicked)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotStartBtnClicked());
}
