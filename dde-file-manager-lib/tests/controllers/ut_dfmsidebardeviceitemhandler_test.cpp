#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define private public
#define protected public

#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
DFM_USE_NAMESPACE

using namespace testing;
namespace  {
    class DFMSideBarDeviceItemHandlerTest : public Test
    {
    public:
        DFMSideBarDeviceItemHandlerTest():Test()
        {
            p_handler = nullptr;
        }

        virtual void SetUp() override {
            p_handler = new DFMSideBarDeviceItemHandler;
        }

        virtual void TearDown() override {
            delete  p_handler;
        }

        DFMSideBarDeviceItemHandler * p_handler;
    };
}

TEST_F(DFMSideBarDeviceItemHandlerTest, create_unmount_action)
{
    ASSERT_NE(p_handler, nullptr);

    DUrl url("Trash");
    bool withTest = true;
    DViewItemAction *action = p_handler->createUnmountOrEjectAction(url, withTest);
    ASSERT_NE(action, nullptr);

    action->deleteLater();
}

TEST_F(DFMSideBarDeviceItemHandlerTest, create_item)
{
    ASSERT_NE(p_handler, nullptr);

    DUrl url("dfmroot:///sda5.localdisk");
    DFMSideBarItem *item = p_handler->createItem(url);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->url(), url);
    delete item;
}

TEST_F(DFMSideBarDeviceItemHandlerTest, cd_action)
{
    ASSERT_NE(p_handler, nullptr);

//    DFileManagerWindow window;
//    const DFMSideBar *bar = window.getLeftSideBar();

//    DUrl url("dfmroot:///sda5.localdisk");
//    DFMSideBarItem *item = p_handler->createItem(url);
//    ASSERT_NE(item, nullptr);

    //线程挂起无法结束
//    p_handler->cdAction(bar, item);

//    delete item;
}

TEST_F(DFMSideBarDeviceItemHandlerTest, context_menu)
{
    ASSERT_NE(p_handler, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();

    DUrl url("dfmroot:///sda5.localdisk");
    DFMSideBarItem *item = p_handler->createItem(url);
    ASSERT_NE(item, nullptr);

    QMenu *menu = p_handler->contextMenu(bar, item);

    EXPECT_NE(menu, nullptr);

    delete item;
    delete menu;
}

TEST_F(DFMSideBarDeviceItemHandlerTest, reset_name)
{
    ASSERT_NE(p_handler, nullptr);

    DUrl url("dfmroot:///sda5.localdisk");
    DFMSideBarItem *item = p_handler->createItem(url);
    ASSERT_NE(item, nullptr);

    p_handler->rename(item, QString("test"));
}
