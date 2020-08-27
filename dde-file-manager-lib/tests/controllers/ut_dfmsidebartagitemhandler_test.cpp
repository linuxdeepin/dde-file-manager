#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define private public
#define protected public

#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"
#include "controllers/dfmsidebartagitemhandler.h"
DFM_USE_NAMESPACE

using namespace testing;
namespace  {
    class DFMSideBarTagItemHandlerTest : public Test
    {
    public:
        DFMSideBarTagItemHandlerTest():Test()
        {
            p_handler = nullptr;
        }

        virtual void SetUp() override {
            p_handler = new DFMSideBarTagItemHandler;
        }

        virtual void TearDown() override {
            delete  p_handler;
        }

        DFMSideBarTagItemHandler * p_handler;
    };
}

TEST_F(DFMSideBarTagItemHandlerTest, create_item)
{
    ASSERT_NE(p_handler, nullptr);

    DUrl url("test");
    DFMSideBarItem *item = p_handler->createItem(url);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->url(), url);
    delete item;
}

TEST_F(DFMSideBarTagItemHandlerTest, context_menu)
{
    ASSERT_NE(p_handler, nullptr);

    DFileManagerWindow window;
    const DFMSideBar *bar = window.getLeftSideBar();

    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
    ASSERT_NE(item, nullptr);

    QMenu *menu = p_handler->contextMenu(bar, item);

    EXPECT_NE(menu, nullptr);

    delete item;
    delete menu;
}

TEST_F(DFMSideBarTagItemHandlerTest, reset_name)
{
    ASSERT_NE(p_handler, nullptr);

    DUrl url("dfmroot:///sda5.localdisk");
    DFMSideBarItem *item = p_handler->createItem(url);
    ASSERT_NE(item, nullptr);

    p_handler->rename(item, QString("test"));
}
