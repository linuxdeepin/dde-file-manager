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

namespace testDFMSideBarItemInterface
{

DFM_USE_NAMESPACE

class DFMSideBarItemInterfaceTest: public testing::Test
{

public:
    DFMSideBarItemInterface *SBIitfs = nullptr;
    DFileManagerWindow* FMWindow = new DFileManagerWindow;
    DFMSideBarItem* item = new DFMSideBarItem;
    DFMSideBar sidebar;

    virtual void SetUp() override
    {
        qDebug() << __PRETTY_FUNCTION__;

        SBIitfs = new DFMSideBarItemInterface();
        item->appendColumn({new QStandardItem("Action1"),
                            new QStandardItem("Action2")});
        item->setText("testAction");
        item->setUrl(QCoreApplication::applicationDirPath());
        item->setGroupName(DFMSideBar::groupName(DFMSideBar::GroupName::Network));
    }

    virtual void TearDown() override
    {
        qDebug() << __PRETTY_FUNCTION__;
    }
};


//in Search-Widget from toolbar,setting the search text have a string from QCoreApplication::applicationDirPath();
TEST_F(DFMSideBarItemInterfaceTest,cdAction)
{
    SBIitfs->cdAction(FMWindow->getLeftSideBar(),item);
    EXPECT_TRUE(0 <= FMWindow->currentUrl().toString().indexOf(QCoreApplication::applicationDirPath()));
//    FMWindow->show(); //should't to show
}

//cause new QMenu class is not empty, set lambda to widget class.
TEST_F(DFMSideBarItemInterfaceTest,contextMenu)
{
    EXPECT_TRUE(SBIitfs->contextMenu(FMWindow->getLeftSideBar(),item));
}

//if can't carsh, nothing to do.
TEST_F(DFMSideBarItemInterfaceTest,rename)
{
    SBIitfs->rename(item,"rename-Funning");
}

}
