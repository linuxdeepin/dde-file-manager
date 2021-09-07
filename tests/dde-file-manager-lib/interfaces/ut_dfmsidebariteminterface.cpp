#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <QProcess>
#include <QApplication>

#include "durl.h"
#include "testhelper.h"
#include "stubext.h"

#include "../views/dfmsidebar.h"
#include "../views/dfilemanagerwindow.h"
#include "../views/dtoolbar.h"

#include "interfaces/dfmsidebaritem.h"
#include "interfaces/dfmsidebariteminterface.h"
#include "interfaces/dfmcrumbbar.h"
#include "interfaces/dfilemenumanager.h"

DFM_USE_NAMESPACE

class TestDFMSideBarItemInterface: public testing::Test
{

public:
    static void SetUpTestCase() {
        stub_ext::StubExt stu;
        stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});
    }

    DFMSideBarItemInterface interface;
    DFileManagerWindow window;

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {
        window.clearActions();
    }
};


//in Search-Widget from toolbar,setting the search text have a string from QCoreApplication::applicationDirPath();
#ifndef __arm__ // arm 下容易死锁 暂时注释
TEST_F(TestDFMSideBarItemInterface,cdAction)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileManagerWindow, cd), [](){return true;});

    DFMSideBarItem* item = new DFMSideBarItem;
    item->appendColumn({ new QStandardItem("Action1"),
                        new QStandardItem("Action2")});

    item->setText("testAction");
    item->setUrl(QCoreApplication::applicationDirPath());
    item->setGroupName(DFMSideBar::groupName(DFMSideBar::GroupName::Network));

    interface.cdAction(window.getLeftSideBar(),item);

    EXPECT_FALSE(0 <= window.currentUrl().toString().indexOf(QCoreApplication::applicationDirPath()));

    //    FMWindow->show(); //should't to show

    if (item) {
        delete item;
        item = nullptr;
    }
}
#endif

//cause new QMenu class is not empty, set lambda to widget class.
#ifndef __arm__ // arm 暂时注释
TEST_F(TestDFMSideBarItemInterface,contextMenu)
{
    DFMSideBarItem* item = new DFMSideBarItem;
    item->setUrl(DUrl(QApplication::applicationDirPath()));
    //interface.contextMenu(window.getLeftSideBar(),item);
    auto menu = interface.contextMenu(window.getLeftSideBar(),item);
    EXPECT_TRUE(menu->actions().size() > 0);

    //to call regiest lambda fuction
    for (auto action : menu->actions()) {
        emit action->triggered(true);
    }

    if (item) {
        delete item;
        item = nullptr;
    }
    delete menu;
    menu = nullptr;
}

//if can't carsh, nothing to do.
TEST_F(TestDFMSideBarItemInterface,rename)
{
    DFMSideBarItem* item = new DFMSideBarItem;
    item->setGroupName("Funning");

    interface.rename(item,"newName");
    EXPECT_TRUE(item->groupName() != "newName");

    if (item) {
        delete item;
        item = nullptr;
    }
}
#endif
