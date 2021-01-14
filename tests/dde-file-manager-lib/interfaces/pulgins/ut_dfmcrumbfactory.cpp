#include <gtest/gtest.h>

#include "interfaces/plugins/dfmcrumbfactory.h"
#include "interfaces/dfmcrumbinterface.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMCrumbFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMCrumbFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMCrumbFactory";
        }
    public:

    };
}

TEST_F(TestDFMCrumbFactory, testCreate)
{
    DFMCrumbInterface *pobject = DFM_NAMESPACE::DFMCrumbFactory::create("video/*");
    EXPECT_EQ(pobject, nullptr);
}

TEST_F(TestDFMCrumbFactory, testKeys)
{
    QStringList lst = DFM_NAMESPACE::DFMCrumbFactory::keys();
    EXPECT_EQ(lst.count(), 0);
}
