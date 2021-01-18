#include <gtest/gtest.h>
#include "controllers/appcontroller.h"

namespace {
class TestAppController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestAppController";
        ctrl = new AppController;
    }

    void TearDown() override
    {
        std::cout << "end TestAppController";
        delete ctrl;
        ctrl = nullptr;
    }

public:
    AppController *ctrl = nullptr;
};
} // namespace

TEST_F(TestAppController, initTest)
{
    ctrl->initConnect();
    ctrl->initControllers();
}
