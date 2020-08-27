#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <interfaces/dfmsidebaritem.h>
#include <views/dfmsidebar.h>
#include <views/dfilemanagerwindow.h>


#define private public
#include "controllers/dfmsidebarvaultitemhandler.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDFMSideBarVaultItemHandler : public testing::Test
    {
    public:
        QSharedPointer<DFMSideBarVaultItemHandler> m_controller;

        virtual void SetUp() override
        {
            m_controller = QSharedPointer<DFMSideBarVaultItemHandler>(new DFMSideBarVaultItemHandler());
            std::cout << "start TestDFMSideBarVaultItemHandler" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMSideBarVaultItemHandler" << std::endl;
        }
    };
}


TEST_F(TestDFMSideBarVaultItemHandler, tst_createItem)
{
//    DFMSideBarItem *item = m_controller->createItem(DFMVAULT_ROOT);
 //   EXPECT_NE(nullptr, item);
 //   delete item;
}

