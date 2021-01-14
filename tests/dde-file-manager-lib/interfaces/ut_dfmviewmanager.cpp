#include "interfaces/dfmviewmanager.h"
#include "views/computerview.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

#define viewManager DFMViewManager::instance()
namespace {
class TestDFMViewManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMSettings";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSettings";
    }
};
}

TEST_F(TestDFMViewManager, dRegisterUrlView)
{
    viewManager->dRegisterUrlView<ComputerView>("computer", "");
    EXPECT_TRUE(viewManager->isRegisted<ComputerView>("computer", ""));
}

TEST_F(TestDFMViewManager, createViewByUrl)
{
//    EXPECT_TRUE(viewManager->createViewByUrl(DUrl(COMPUTER_ROOT)) != nullptr);
}

TEST_F(TestDFMViewManager, suitedViewTypeNameByUrl)
{
    EXPECT_FALSE(viewManager->suitedViewTypeNameByUrl(DUrl(COMPUTER_ROOT)).isEmpty());
}

TEST_F(TestDFMViewManager, clearUrlView)
{
    viewManager->clearUrlView("computer", "");
    EXPECT_FALSE(viewManager->isRegisted<ComputerView>("computer", ""));
}
