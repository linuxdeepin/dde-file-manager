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


#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfilemenumanager.h"
#include "singleton.h"
#include <dfmevent.h>
#include "testhelper.h"
#include "stub-ext/stubext.h"

#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUrlQuery>
#include <gtest/gtest.h>

#define private public
#include "controllers/bookmarkmanager.h"


#define BOOKMARK_STR "TestBookMark"
#define BOOKMARK_NEW_STR "TestNewBookMark"

DFM_USE_NAMESPACE

namespace  {
    class TestBookMarkManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = Singleton<BookMarkManager>::instance();
            m_pHandler = new DFMSideBarBookmarkItemHandler();
            std::cout << "start TestBookMarkManager";
            tempDirPath =  QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/" + BOOKMARK_STR;
        }
        void TearDown() override
        {
            if (m_pItem) {
                delete m_pItem;
                m_pItem = nullptr;
            }

            delete m_pHandler;
            m_pHandler = nullptr;

            m_pTester = nullptr;
            std::cout << "end TestBookMarkManager";
        }
    public:
        BookMarkManager *m_pTester = nullptr;
        DFMSideBarBookmarkItemHandler * m_pHandler = nullptr;
        DFMSideBarItem *m_pItem = nullptr;
        QString tempDirPath;
    };
}

TEST_F(TestBookMarkManager, can_touch_a_new_bookmark)
{
    QProcess::execute("mkdir " + tempDirPath);

    DUrl fileUrl = DUrl::fromLocalFile(tempDirPath);
    QStorageInfo si(tempDirPath);
    QList<QPair<QString, QString>> queryItems;
    queryItems.append(QPair<QString, QString>("mount_point", "device:" + QVariant(si.device()).toString()));
    queryItems.append(QPair<QString, QString>("locate_url", tempDirPath.replace("/home/", "/")));
    QUrlQuery query;
    query.setQueryItems(queryItems);

    DUrl bookMarkFrom = DUrl::fromBookMarkFile(fileUrl, fileUrl.fileName());
    bookMarkFrom.setQuery(query);
    QSharedPointer<DFMTouchFileEvent> event(new DFMTouchFileEvent(nullptr, bookMarkFrom));

    EXPECT_TRUE(m_pTester->touch(event));
}

TEST_F(TestBookMarkManager, can_create_bookmark_sidebar_item)
{
    ASSERT_NE(m_pHandler, nullptr);

    DUrl url = DUrl::fromBookMarkFile(DUrl("file://" + tempDirPath), BOOKMARK_STR);
    m_pItem = m_pHandler->createItem(url);
    ASSERT_NE(m_pItem, nullptr);
    EXPECT_EQ(m_pItem->url(), url);
}

//TEST_F(TestBookMarkManager, can_call_cd_action)
//{
//    ASSERT_NE(m_pHandler, nullptr);

//    DFileManagerWindow window;
//    const DFMSideBar *bar = window.getLeftSideBar();
//    DUrl url = DUrl::fromBookMarkFile(DUrl("file://" + tempDirPath), BOOKMARK_STR);
//    m_pItem = m_pHandler->createItem(url);
//    ASSERT_NE(m_pItem, nullptr);

//    m_pHandler->cdAction(bar, m_pItem);
//}

TEST_F(TestBookMarkManager, can_call_context_menu)
{
    ASSERT_NE(m_pHandler, nullptr);

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

    DFileManagerWindow *window = new DFileManagerWindow();
    const DFMSideBar *bar = window->getLeftSideBar();
    DUrl url = DUrl::fromBookMarkFile(DUrl("file://" + tempDirPath), BOOKMARK_STR);
    m_pItem = m_pHandler->createItem(url);
    ASSERT_NE(m_pItem, nullptr);

    QMenu *menu = m_pHandler->contextMenu(bar, m_pItem);

    EXPECT_NE(menu, nullptr);
    delete menu;

    window->clearActions();
    FreePointer(window);
}

TEST_F(TestBookMarkManager, can_rename_bookmark_sidebar_item)
{
    ASSERT_NE(m_pHandler, nullptr);

    DUrl url = DUrl::fromBookMarkFile(DUrl("file://" + tempDirPath), BOOKMARK_STR);
    m_pItem = m_pHandler->createItem(url);
    ASSERT_NE(m_pItem, nullptr);

    m_pHandler->rename(m_pItem, BOOKMARK_NEW_STR);
}

TEST_F(TestBookMarkManager, can_rename_bookmark)
{
    DUrl fileUrl = DUrl::fromLocalFile(tempDirPath);
    DUrl bookMarkFrom = DUrl::fromBookMarkFile(fileUrl, BOOKMARK_STR);
    DUrl bookMarkTo = DUrl::fromBookMarkFile(fileUrl, BOOKMARK_NEW_STR);

    QSharedPointer<DFMRenameEvent> event(new DFMRenameEvent(nullptr, bookMarkFrom, bookMarkTo));

    EXPECT_TRUE(m_pTester->renameFile(event));
}

TEST_F(TestBookMarkManager, can_rename_bookmark_file)
{
    DUrl fromUrl = DUrl::fromLocalFile(tempDirPath);
    DUrl toUrl = DUrl::fromLocalFile(tempDirPath.replace(BOOKMARK_STR, BOOKMARK_NEW_STR));

    EXPECT_TRUE(m_pTester->onFileRenamed(fromUrl, toUrl));
}

TEST_F(TestBookMarkManager, can_find_bookmark_data)
{
    DUrl url = DUrl::fromLocalFile(tempDirPath.replace(BOOKMARK_STR, BOOKMARK_NEW_STR));

    EXPECT_NO_FATAL_FAILURE(m_pTester->findBookmarkData(url));
}

TEST_F(TestBookMarkManager, can_refresh_bookmark)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->refreshBookmark());
}

TEST_F(TestBookMarkManager, can_get_bookmark_urls)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->getBookmarkUrls());
}

TEST_F(TestBookMarkManager, can_remove_bookmark)
{
    DUrl fileUrl = DUrl::fromBookMarkFile(DUrl::fromLocalFile(tempDirPath.replace(BOOKMARK_STR, BOOKMARK_NEW_STR)), BOOKMARK_NEW_STR);

    QSharedPointer<DFMDeleteEvent> event(new DFMDeleteEvent(nullptr, DUrlList{fileUrl}));

    EXPECT_TRUE(m_pTester->deleteFiles(event));

    QProcess::execute("rm -rf " + tempDirPath);
}
