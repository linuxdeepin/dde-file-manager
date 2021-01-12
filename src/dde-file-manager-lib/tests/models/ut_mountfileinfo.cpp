#include "models/mountfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestMountFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMountFileInfo";
        info = new MountFileInfo(DUrl("/"));
    }

    void TearDown() override
    {
        std::cout << "end TestMountFileInfo";
    }

public:
    MountFileInfo *info;
};
} // namespace

TEST_F(TestMountFileInfo, canFetch)
{
    EXPECT_TRUE(info->canFetch());
}
