/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hujianzhong<hujianzhong@uniontech.com>
 *
 * Maintainer: hujianzhong<hujianzhong@uniontech.com>
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

#define FreePtr(x) if(x){delete x;x=nullptr;}

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
        menu = new DFileMenu();
        menu->setAccessibleInfo("test");
        QObject::connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
        myAction = new QAction();
        menu->addAction(myAction);


        DUrl url("file:///home");
        DUrlList urllist;
        urllist.append(DUrl::fromLocalFile("file:///home"));
        urllist.append(DUrl::fromLocalFile("file:///tmp"));
        QObject *sender = new QObject(menu);
        QModelIndex index;
        menu->setEventData(url, urllist, eventId, sender, index);
        sender->deleteLater();
        std::cout << "start TestDFileMenu";
    }

    void TearDown() override
    {
        menu->deleteLater();
        myAction->deleteLater();
        std::cout << "end TestDFileMenu";
    }
};
}
#if 0
TEST_F(TestDFileMenu, tst_makeEvent)
{
    TestHelper::runInLoop([]() {});
    MenuAction type;
    type = MenuAction::Open;
    const QSharedPointer<DFMMenuActionEvent> &event = menu->makeEvent(type);

}
#endif
TEST_F(TestDFileMenu, setEventData)
{
    DUrl url("file:///home");
    DUrlList urllist;
    urllist.append(DUrl::fromLocalFile("file:///home"));
    urllist.append(DUrl::fromLocalFile("file:///tmp"));
    DFileMenu *menu = new DFileMenu;
    QWidget *w = new QWidget();
    menu->setProperty("DFileManagerWindow", (quintptr)this);
    menu->setProperty("ToolBarSettingsMenu", true);
    menu->setEventData(DUrl(), DUrlList() << DUrl(), w->winId(), w);
    delete  w;
    delete menu;
}
TEST_F(TestDFileMenu, actionAt)
{
    EXPECT_EQ(myAction, menu->actionAt(0));
}

TEST_F(TestDFileMenu, actionAt_text)
{
    menu->actionAt(actionName);
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
    FreePtr(event);
}
