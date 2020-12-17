#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/pathmanager.h"

#include "stub.h"

DFM_USE_NAMESPACE

using namespace testing;
namespace  {
    class DFMSideBarDefaultItemHandlerTest : public Test
    {
    public:
        DFMSideBarDefaultItemHandlerTest():Test()
        {
            p_handler = nullptr;
        }

        virtual void SetUp() override {
            p_handler = new DFMSideBarDefaultItemHandler;
        }

        virtual void TearDown() override {
            delete  p_handler;
        }

        DFMSideBarDefaultItemHandler * p_handler;
    };
}

TEST_F(DFMSideBarDefaultItemHandlerTest, create_item)
{
    ASSERT_NE(p_handler, nullptr);
    QString pathKey("Trash");
    DFMSideBarItem *item = p_handler->createItem(pathKey);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->url().toString(), QString(TRASH_ROOT));
    delete item;
}

TEST_F(DFMSideBarDefaultItemHandlerTest, cd_action)
{
    ASSERT_NE(p_handler, nullptr);

    Stub stub;
    static bool myCallOpen = false;
    void (*ut_openNewTab)() = [](){myCallOpen = true;};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;
    EXPECT_TRUE(myCallOpen);

    const DFMSideBar *bar = window.getLeftSideBar();

    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("test"));
    ASSERT_NE(item, nullptr);

    p_handler->cdAction(bar, item);

    delete item;
}

TEST_F(DFMSideBarDefaultItemHandlerTest, context_menu)
{
    Stub stub;
    static bool myCallOpen = false;
    void (*ut_openNewTab)() = [](){myCallOpen = true;};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;
    EXPECT_TRUE(myCallOpen);

    const DFMSideBar *bar = window.getLeftSideBar();

    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
    ASSERT_NE(item, nullptr);

    QString (*st_getSystemPathDisplayName)(QString) = [](QString) {
        return QString("displayName");
    };

    stub.set(ADDR(PathManager, getSystemPathDisplayName), st_getSystemPathDisplayName);

    QMenu *menu = p_handler->contextMenu(bar, item);
    EXPECT_NE(menu, nullptr);
    QList<QAction *> actions = menu->actions();
    for (auto action : actions) {
        action->trigger();
    }

    void (*st_actionClearTrash)(const QObject *) = [](const QObject *){
        // do nothing.
    };
    stub.set(ADDR(AppController, actionClearTrash), st_actionClearTrash);

    QString (*st_getSystemPathDisplayName2)(QString) = [](QString) {
        return QString("");
    };
    stub.set(ADDR(PathManager, getSystemPathDisplayName), st_getSystemPathDisplayName2);

    menu = p_handler->contextMenu(bar, item);
    EXPECT_NE(menu, nullptr);
    actions = menu->actions();
    for (auto action : actions) {
        action->trigger();
    }

    delete item;
    delete menu;
}
