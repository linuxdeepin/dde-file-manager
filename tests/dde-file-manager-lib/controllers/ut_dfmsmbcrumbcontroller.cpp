#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


#define private public
#include "controllers/dfmsmbcrumbcontroller.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDFMSmbCrumbController : public testing::Test
    {
    public:
        QSharedPointer<DFMSmbCrumbController> m_controller;

        virtual void SetUp() override
        {
            m_controller = QSharedPointer<DFMSmbCrumbController>(new DFMSmbCrumbController());
            std::cout << "start TestDFMSmbCrumbController" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMSmbCrumbController" << std::endl;
        }
    };
}


TEST_F(TestDFMSmbCrumbController, tst_supportedUrl)
{
    EXPECT_TRUE(m_controller->supportedUrl(DUrl::fromSMBFile("smb/path")));
    EXPECT_FALSE(m_controller->supportedUrl(DUrl::fromComputerFile("Videos")));
}

