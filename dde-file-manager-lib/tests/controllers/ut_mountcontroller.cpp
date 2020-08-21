#include <gtest/gtest.h>
#include "controllers/mountcontroller.h"
#include "dfmevent.h"

namespace {
class TestMountController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMountController";
        controller = new MountController;
    }

    void TearDown() override
    {
        std::cout << "end TestMountController";
        delete controller;
        controller = nullptr;
    }

public:
    MountController *controller;
};
} // namespace

TEST_F(TestMountController, createFileInfo)
{
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(controller->createFileInfo(event) != nullptr);
}

TEST_F(TestMountController, getChildren)
{
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("file:///"), QStringList(), QDir::AllDirs);
    EXPECT_TRUE(controller->getChildren(event).count() == 0);
}
