#include <gtest/gtest.h>
#include "client/filemanagerclient.h"

namespace {
class TestFileManagerClient : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestFileManagerClient";
        cli = new FileManagerClient;
    }

    void TearDown() override
    {
        std::cout << "end TestFileManagerClient";
        delete cli;
        cli = nullptr;
    }

public:
    FileManagerClient *cli = nullptr;
};
} // namespace

TEST_F(TestFileManagerClient, testNewNameJob)
{
    cli->testNewNameJob("oldFile", "newFile");
}
