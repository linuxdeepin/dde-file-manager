#include <gtest/gtest.h>

#include "interfaces/plugins/dfmfilecontrollerfactory.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMFileControllerFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMFileControllerFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMFileControllerFactory";
        }
    public:
    };
}

TEST_F(TestDFMFileControllerFactory, testCreate)
{
    DAbstractFileController *pr = DFMFileControllerFactory::create("video/*");
    EXPECT_EQ(pr, nullptr);
}

TEST_F(TestDFMFileControllerFactory, testKeys)
{
    QStringList lst = DFMFileControllerFactory::keys();
    EXPECT_EQ(lst.count(), 0);
}
