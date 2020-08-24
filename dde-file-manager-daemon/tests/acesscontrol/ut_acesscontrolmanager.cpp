#include <gtest/gtest.h>
#include "acesscontrol/acesscontrolmanager.h"

namespace {
class TestAcessControlManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestAcessControlManager";
        mng = new AcessControlManager;
    }

    void TearDown() override
    {
        std::cout << "end TestAcessControlManager";
        delete mng;
        mng = nullptr;
    }

public:
    AcessControlManager *mng = nullptr;
};
} // namespace

TEST_F(TestAcessControlManager, coverageTest)
{
    mng->initConnect();
}
