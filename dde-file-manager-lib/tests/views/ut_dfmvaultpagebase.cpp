#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QWindow>

#define private public
#include "views/dfmvaultactiveview.h"


namespace  {
    class TestDFMVaultPageBase : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultPageBase> m_view;

        virtual void SetUp() override
        {
            m_view = QSharedPointer<DFMVaultPageBase>(new DFMVaultPageBase());
            m_view->show();
            std::cout << "start TestDFMVaultPageBase" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultPageBase" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultPageBase, tst_set_get_wndPtr)
{
   QSharedPointer<QWidget> window = QSharedPointer<QWidget>(new QWidget());
   m_view->setWndPtr(window.get());
   EXPECT_EQ(window, m_view->getWndPtr());
}

TEST_F(TestDFMVaultPageBase, tst_showTop)
{
   m_view->showTop();
   EXPECT_FALSE(m_view->isHidden());
   EXPECT_TRUE(m_view->isTopLevel());
}
