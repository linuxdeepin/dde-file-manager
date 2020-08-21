#include <gtest/gtest.h>
#include "controllers/fileoperation.h"

namespace {
class TestFileOperation : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestFileOperation";
    }

    void TearDown() override
    {
        std::cout << "end TestFileOperation";
    }

public:
};
} // namespace
