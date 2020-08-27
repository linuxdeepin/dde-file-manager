#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "views/dfmvaultactiveview.h"


namespace  {
    class TestDFMVaultActiveView : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultActiveView> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultActiveView>(new DFMVaultActiveView());
            m_view->show();
            std::cout << "start TestDFMVaultActiveView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveView, tst_slotNextWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotNextWidget());
}
