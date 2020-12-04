#include <gtest/gtest.h>

#include "interfaces/plugins/dfmcrumbfactory.h"

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
    DFM_NAMESPACE::DFMCrumbFactory::create("video/*");
}

TEST_F(TestDFMCrumbFactory, testKeys)
{
    DFM_NAMESPACE::DFMCrumbFactory::keys();
}
