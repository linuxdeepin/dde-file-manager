#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <interfaces/dfmsidebaritem.h>
#include <views/dfmsidebar.h>
#include <views/dfilemanagerwindow.h>
#include "controllers/vaultcontroller.h"

#define private public
#include "controllers/dfmvaultcrumbcontroller.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDFMVaultCrumbController : public testing::Test
    {
    public:
        QSharedPointer<DFMVaultCrumbController> m_controller;

        virtual void SetUp() override
        {
            m_controller = QSharedPointer<DFMVaultCrumbController>(new DFMVaultCrumbController());
            std::cout << "start TestDFMVaultCrumbController" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMVaultCrumbController" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultCrumbController, tst_createItem)
{
    EXPECT_TRUE(m_controller->supportedUrl(VaultController::makeVaultUrl()));
    EXPECT_FALSE(m_controller->supportedUrl(DUrl::fromComputerFile("Videos")));
}

TEST_F(TestDFMVaultCrumbController, tst_seprateUrl)
{
    QList<CrumbData> ret = m_controller->seprateUrl(VaultController::makeVaultUrl(VaultController::makeVaultLocalPath()));
    EXPECT_TRUE(ret.size() > 0);
}
