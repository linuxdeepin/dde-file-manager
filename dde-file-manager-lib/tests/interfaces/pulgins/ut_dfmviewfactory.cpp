#include <gtest/gtest.h>

#include "interfaces/plugins/dfmviewfactory.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMViewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMViewFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMViewFactory";
        }
    public:
    };
}

TEST_F(TestDFMViewFactory, testCreate)
{
    DFMViewFactory::create("/views");
}

TEST_F(TestDFMViewFactory, testKeys)
{
    DFMViewFactory::keys();
}

TEST_F(TestDFMViewFactory, testViewIsSuitedWithUrl)
{
    DFMViewFactory::viewIsSuitedWithUrl(nullptr, DUrl("file:///utest"));
}
