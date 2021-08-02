/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_DEFAULT));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_DEVICE));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_TAG));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_BOOKMARK));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(SIDEBAR_ID_VAULT));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral("tInterface_first"));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral("tInterface_secoed"));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral(""));
        EXPECT_TRUE(nullptr != interface);
        delete interface;
        interface = nullptr;
    }
    {
        auto interface = manager->createByIdentifier(QStringLiteral("error-code"));
        EXPECT_TRUE(nullptr == interface);
        delete interface;
        interface = nullptr;
    }
}
