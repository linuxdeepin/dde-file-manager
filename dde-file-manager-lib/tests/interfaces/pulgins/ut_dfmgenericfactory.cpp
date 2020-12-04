
#include <gtest/gtest.h>

#include "interfaces/plugins/dfmgenericfactory.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMGenericFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMGenericFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMGenericFactory";
        }
    public:

    };
}

TEST_F(TestDFMGenericFactory, testCreate)
{
    DFMGenericFactory::create("video/*");
}

TEST_F(TestDFMGenericFactory, testcreateAll)
{
    DFMGenericFactory::createAll("video/*");
}

TEST_F(TestDFMGenericFactory, testKeys)
{
    DFMGenericFactory::keys();
}
