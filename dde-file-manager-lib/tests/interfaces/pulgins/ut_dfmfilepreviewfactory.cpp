#include <gtest/gtest.h>

#include "interfaces/plugins/dfmfilepreviewfactory.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMFilePreviewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMFilePreviewFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMFilePreviewFactory";
        }
    public:
    };
}

TEST_F(TestDFMFilePreviewFactory, testCreate)
{
//    DFMFilePreviewFactory::create("/previews");
}


TEST_F(TestDFMFilePreviewFactory, testKeys)
{
//    DFMFilePreviewFactory::keys();
}

TEST_F(TestDFMFilePreviewFactory, testIsSuitedWithKey)
{
    QString index("");
    DFMFilePreviewFactory::isSuitedWithKey(nullptr, index);
}
