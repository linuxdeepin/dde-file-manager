#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <QProcess>
//#include <QStanardItem>

#include "durl.h"

#include "../views/dfmsidebar.h"
#include "../views/dfilemanagerwindow.h"
#include "../views/dtoolbar.h"

#include "interfaces/dfmsidebaritem.h"
#include "interfaces/dfmsidebariteminterface.h"
#include "interfaces/dfmcrumbbar.h"

DFM_USE_NAMESPACE

class TestDFMSideBarItemInterface: public testing::Test
{

public:
    DFMSideBarItemInterface interface;
    DFileManagerWindow window;
    DFMSideBarItem item;

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {

    }
};


//in Search-Widget from toolbar,setting the search text have a string from QCoreApplication::applicationDirPath();
TEST_F(TestDFMSideBarItemInterface,cdAction)
{
    item.appendColumn({ new QStandardItem("Action1"),
                        new QStandardItem("Action2")});

    item.setText("testAction");
    item.setUrl(QCoreApplication::applicationDirPath());
    item.setGroupName(DFMSideBar::groupName(DFMSideBar::GroupName::Network));

    interface.cdAction(window.getLeftSideBar(),&item);

    EXPECT_TRUE(0 <= window.currentUrl().toString().indexOf(QCoreApplication::applicationDirPath()));
    //    FMWindow->show(); //should't to show
}

//cause new QMenu class is not empty, set lambda to widget class.
TEST_F(TestDFMSideBarItemInterface,contextMenu)
{
    interface.contextMenu(window.getLeftSideBar(),&item);
    auto menu = interface.contextMenu(window.getLeftSideBar(),&item);
    EXPECT_TRUE(menu->actions().size() > 0);

    for (auto action : menu->actions()) {
        emit action->triggered(); //to call regiest lambda fuction
    }
}

//if can't carsh, nothing to do.
TEST_F(DFMSideBarItemInterfaceTest,rename)
{
    interface.rename(&item,"newName");
    EXPECT_TRUE(item.groupName() != "newName");
}
