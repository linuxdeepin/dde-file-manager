#include <gtest/gtest.h>

#include "interfaces/dfmsidebariteminterface.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "controllers/dfmsidebarvaultitemhandler.h"
#include "interfaces/dfmsidebarmanager.h"


DFM_USE_NAMESPACE

//测试的Test
class TestInterfacePrivate: public DFMSideBarItemInterface
{

};

class TestDFMSideBaManager: public testing::Test
{

public:
    DFMSideBarManager* manager = nullptr;

    virtual void SetUp() override
    {
        if (nullptr == manager) {
            manager = DFMSideBarManager::instance();
        }
    }

    virtual void TearDown() override
    {

    }
};

TEST_F(TestDFMSideBaManager,dRegisterSideBarInterface)
{

    //使用测试
    EXPECT_TRUE(manager->isRegisted<DFMSideBarItemInterface>(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK)));
    EXPECT_TRUE(manager->isRegisted<DFMSideBarDefaultItemHandler>(QStringLiteral(SIDEBAR_ID_DEFAULT)));
    EXPECT_TRUE(manager->isRegisted<DFMSideBarDeviceItemHandler>(QStringLiteral(SIDEBAR_ID_DEVICE)));
    EXPECT_TRUE(manager->isRegisted<DFMSideBarTagItemHandler>(QStringLiteral(SIDEBAR_ID_TAG)));
    EXPECT_TRUE(manager->isRegisted<DFMSideBarBookmarkItemHandler>(QStringLiteral(SIDEBAR_ID_BOOKMARK)));
    EXPECT_TRUE(manager->isRegisted<DFMSideBarVaultItemHandler>(QStringLiteral(SIDEBAR_ID_VAULT)));


    //反例 非继承interface 注册 测试结果为不能编译 符合预期
    //    struct testNotInh{QString name;};
    //    struct testNotInh emptyFirst{QString("emptyFirst")};
    //    struct testNotInh emptySecoed{QString("emptySecoed")};
    //    sbManager->dRegisterSideBarInterface<testNotInh>(emptyFirst.name);
    //    EXPECT_TRUE(!sbManager->isRegisted<testNotInh>(emptyFirst.name));

    //    sbManager->dRegisterSideBarInterface<testNotInh>(emptyFirst.name);
    //    EXPECT_TRUE(!sbManager->isRegisted<testNotInh>(emptyFirst.name));


    //注册 First Second 空索引 hash
    TestInterfacePrivate first,secoed,three;
    manager->dRegisterSideBarInterface<TestInterfacePrivate>("tInterface_first");
    EXPECT_TRUE(manager->isRegisted<TestInterfacePrivate>("tInterface_first"));

    manager->dRegisterSideBarInterface<TestInterfacePrivate>("tInterface_secoed");
    EXPECT_TRUE(manager->isRegisted<TestInterfacePrivate>("tInterface_secoed"));

    manager->dRegisterSideBarInterface<TestInterfacePrivate>("");
    EXPECT_TRUE(manager->isRegisted<TestInterfacePrivate>(""));

    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_DEFAULT)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_DEVICE)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_TAG)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_BOOKMARK)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_VAULT)));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral("tInterface_first")));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral("tInterface_secoed")));
    EXPECT_TRUE(nullptr != manager->createByIdentifier(QStringLiteral("")));
    EXPECT_TRUE(nullptr == manager->createByIdentifier(QStringLiteral("error-code")));
}
