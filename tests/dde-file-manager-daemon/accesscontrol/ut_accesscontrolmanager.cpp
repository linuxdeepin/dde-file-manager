#include <gtest/gtest.h>
#include "accesscontrol/accesscontrolmanager.h"

namespace {
class TestAccessControlManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestAccessControlManager";
        mng = new AccessControlManager;
    }

    void TearDown() override
    {
        std::cout << "end TestAccessControlManager";
        delete mng;
        mng = nullptr;
    }

public:
    AccessControlManager *mng = nullptr;
};
} // namespace

TEST_F(TestAccessControlManager, coverageTest)
{
    mng->initConnect();
}
