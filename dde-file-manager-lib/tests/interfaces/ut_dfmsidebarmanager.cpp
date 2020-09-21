#include "interfaces/dfmsidebarmanager.h"
#include "views/computerview.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebarvaultitemhandler.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

#define viewManager DFMViewManager::instance()
namespace {
class TestDFMSideBarManager : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMSideBarManager";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSideBarManager";
    }
};
}

TEST_F(TestDFMSideBarManager, isRegisted)
{
    DFMSideBarManager::instance()->dRegisterSideBarInterface<DFMSideBarItemInterface>(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK));
    EXPECT_TRUE(DFMSideBarManager::instance()->isRegisted<DFMSideBarItemInterface>(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK)));
}

TEST_F(TestDFMSideBarManager, createByIdentifier)
{
    EXPECT_TRUE(DFMSideBarManager::instance()->createByIdentifier("__default") != nullptr);
}
TEST_F(TestDFMSideBarManager, createByIdentifier2)
{
    EXPECT_TRUE(DFMSideBarManager::instance()->createByIdentifier("test") == nullptr);
}
