#include <gtest/gtest.h>
#include "controllers/masteredmediacontroller.h"

namespace {
class TestMasteredMediaController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMasteredMediaController";
    }

    void TearDown() override
    {
        std::cout << "end TestMasteredMediaController";
    }

public:
};
} // namespace
