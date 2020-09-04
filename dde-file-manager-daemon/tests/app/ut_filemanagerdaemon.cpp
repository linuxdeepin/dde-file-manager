#include <gtest/gtest.h>
#include "app/filemanagerdaemon.h"

namespace {
class TestFileManagerDaemon : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestFileManagerDaemon";
        daemon = new FileManagerDaemon;
    }

    void TearDown() override
    {
        std::cout << "end TestFileManagerDaemon";
        delete daemon;
        daemon = nullptr;
    }

public:
    FileManagerDaemon *daemon = nullptr;
};
} // namespace

TEST_F(TestFileManagerDaemon, coverageTest)
{
    daemon->initControllers();
    daemon->initConnect();
}
