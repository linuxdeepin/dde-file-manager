#include <gtest/gtest.h>
#include "controllers/mergeddesktopcontroller.h"

namespace {
class TestMergedDesktopController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMergedDesktopController";
    }

    void TearDown() override
    {
        std::cout << "end TestMergedDesktopController";
    }

public:
};
} // namespace
