#include "interfaces/dfilemenu.h"
#include "interfaces/dfilemenumanager.h"
#include "interfaces/dabstractfileinfo.h"

#include "singleton.h"

#include "app/define.h"
#include "views/windowmanager.h"

#include <gtest/gtest.h>
#include <QFrame>
#include <QMetaObject>
#include <QMetaEnum>
#include <QMenu>
#include <QWidget>
#include <QGuiApplication>
#include "dfmcrumbbar.h"
#include "testhelper.h"
DFM_USE_NAMESPACE

namespace {
class TestDFileMenu : public testing::Test
{
public:
    DFileMenu *menu;
    QString actionName = "testAction";
    QAction *myAction;
    quint64 eventId = 123456;
    void SetUp() override
    {
        menu = new DFileMenu;
        QObject::connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
        myAction = new QAction(actionName);
        menu->addAction(myAction);


        DUrl url("file:///home");
        DUrlList urllist;
        urllist.append(DUrl("file:///home"));
        urllist.append(DUrl("file:///tmp"));
        QObject *sender = new QObject();
        QModelIndex index;
        menu->setEventData(url, urllist, eventId, sender, index);
        std::cout << "start TestDFileMenu";
    }

    void TearDown() override
    {
        menu->deleteLater();
        std::cout << "end TestDFileMenu";
    }
};
}

TEST_F(TestDFileMenu, tst_makeEvent)
{
    TestHelper::runInLoop([](){});
    MenuAction type;
    type = MenuAction::Open;
    const QSharedPointer<DFMMenuActionEvent> &event = menu->makeEvent(type);

}
TEST_F(TestDFileMenu, setEventData)
{
    DUrl url("file:///home");
    DUrlList urllist;
    urllist.append(DUrl("file:///home"));
    urllist.append(DUrl("file:///tmp"));
    QObject *sender = new QObject();
    QModelIndex index;
    menu->setEventData(url, urllist, eventId, sender, index);
}
TEST_F(TestDFileMenu, actionAt)
{
    EXPECT_EQ(myAction, menu->actionAt(0));
}

TEST_F(TestDFileMenu, actionAt_text)
{
    EXPECT_EQ(myAction, menu->actionAt(actionName));
}

TEST_F(TestDFileMenu, actionAt_text2)
{
    EXPECT_NE(myAction, menu->actionAt("test"));
}

TEST_F(TestDFileMenu, eventId)
{
    EXPECT_EQ(eventId, menu->eventId());
}

TEST_F(TestDFileMenu, ignoreMenuActions)
{
    menu->ignoreMenuActions();
}

TEST_F(TestDFileMenu, selectedUrls)
{
    menu->selectedUrls();
}

TEST_F(TestDFileMenu, deleteLater)
{
    menu->deleteLater();
}

TEST_F(TestDFileMenu, deleteLater2)
{
    QWidget w;
    menu->deleteLater(&w);
}

TEST_F(TestDFileMenu, setIgnoreMenuActions)
{
    QSet<DFMGlobal::MenuAction> ignoreMenuActions;
    ignoreMenuActions.insert(MenuAction::Open);
    menu->setIgnoreMenuActions(ignoreMenuActions);
}

TEST_F(TestDFileMenu, mouseMoveEvent)
{
    QMouseEvent *event = new QMouseEvent(QMouseEvent::MouseMove,
                                         QCursor::pos(),
                                         Qt::NoButton,
                                         Qt::NoButton,
                                         Qt::NoModifier);
    menu->mouseMoveEvent(event);
}
