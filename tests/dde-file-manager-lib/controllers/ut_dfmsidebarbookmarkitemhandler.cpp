#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "stubext.h"
#include "addr_pri.h"
#include "testhelper.h"

#include "app/define.h"
#include "singleton.h"
#include "dfmevent.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfmsidebaritem.h"
#include "dialogs/dialogmanager.h"
#include "views/windowmanager.h"
#include "views/dfmsidebar.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#define private public
#include "controllers/bookmarkmanager.h"
#include "interfaces/dfilemenumanager.h"

#include <DDialog>

#include <QScopedPointer>
#include <QSharedPointer>
#include <QDialog>

using namespace stub_ext;

using namespace testing;
namespace  {
    class TestDFMSideBarBookmarkItemHandler : public Test
    {
    public:
        virtual void SetUp() override {
            m_handler = new DFMSideBarBookmarkItemHandler();

            m_dirPath = TestHelper::createTmpDir();
        }

        virtual void TearDown() override {
            if (m_handler)
            {
                delete  m_handler;
                m_handler = nullptr;
            }

            TestHelper::deleteTmpFile(m_dirPath);
        }

        DFMSideBarBookmarkItemHandler *m_handler = nullptr;
        QString m_dirPath;
    };
}

TEST_F(TestDFMSideBarBookmarkItemHandler, test_createItem)
{
    ASSERT_NE(m_handler, nullptr);

    DUrl dirUrl = DUrl::fromLocalFile(m_dirPath);

    QScopedPointer<DFMSideBarItem> pItem(m_handler->createItem(dirUrl));
    EXPECT_NE(pItem.data(), nullptr);
}

TEST_F(TestDFMSideBarBookmarkItemHandler, test_cdAction)
{
    ASSERT_NE(m_handler, nullptr);

    StubExt st;
    st.set_lamda(&DFileManagerWindow::openNewTab, [](){return true;});

    st.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

    DFileManagerWindow *window = new DFileManagerWindow();
    const DFMSideBar *bar = window->getLeftSideBar();
    DUrl dirUrl = DUrl::fromLocalFile(m_dirPath);
    QScopedPointer<DFMSideBarItem> pItem_1(m_handler->createItem(dirUrl));

    bool called = false;
    st.set_lamda(&DFileManagerWindow::cd, [&](){called = true; return true;});
    m_handler->cdAction(bar, pItem_1.data());
    EXPECT_TRUE(called);

    called = false;
    QScopedPointer<DFMSideBarItem> pItem_2(m_handler->createItem(DUrl::fromLocalFile(m_dirPath + "_not_exists")));
    st.set_lamda(&DialogManager::showRemoveBookMarkDialog, [](){return QDialog::Accepted;});
    st.set_lamda(&DFileService::deleteFiles, [&](){called = true; return true;});
    m_handler->cdAction(bar, pItem_2.data());
    EXPECT_TRUE(called);

    window->clearActions();
    FreePointer(window);
}

TEST_F(TestDFMSideBarBookmarkItemHandler, test_contextMenu)
{
    ASSERT_NE(m_handler, nullptr);

    int calledActionCount = 0;
    StubExt st;
    st.set_lamda(&DFileManagerWindow::openNewTab, [&](){++calledActionCount; return true;});
    st.set_lamda(&WindowManager::showNewWindow, [&](){++calledActionCount;});
    st.set_lamda(&DFMSideBar::openItemEditor, [&](){++calledActionCount;});
    st.set_lamda(&DialogManager::showPropertyDialog, [&](){++calledActionCount;});
    st.set_lamda(&DFileService::deleteFiles, [&](){++calledActionCount; return true;});
    st.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

    st.set_lamda(VADDR(DDialog, exec), [&]{++calledActionCount; return 0;});

    DFileManagerWindow *window = new DFileManagerWindow();
    const DFMSideBar *bar = window->getLeftSideBar();
    DUrl dirUrl = DUrl::fromBookMarkFile(DUrl(m_dirPath), "Test_Bookmark");

    BookMarkPointer pBM(new BookMark(dirUrl));
    QUrlQuery query(dirUrl);
    pBM->m_created = QDateTime::currentDateTime();
    pBM->m_lastModified = QDateTime::currentDateTime();
    pBM->mountPoint = query.queryItemValue("mount_point");
    pBM->locateUrl = query.queryItemValue("locate_url");

    QScopedPointer<DFMSideBarItem> pItem(m_handler->createItem(dirUrl));
    QScopedPointer<QMenu> pMenu(m_handler->contextMenu(bar, pItem.data()));

    calledActionCount = 0;
    QList<QAction *> actions = pMenu->actions();
    EXPECT_GE(pMenu->actions().count(), 5);

    for (auto action : actions) {
        action->trigger();
    }
    EXPECT_GE(calledActionCount, 5);

    window->clearActions();
    FreePointer(window);
}

TEST_F(TestDFMSideBarBookmarkItemHandler, test_rename)
{
    ASSERT_NE(m_handler, nullptr);

    DUrl dirUrl = DUrl::fromLocalFile(m_dirPath);
    QScopedPointer<DFMSideBarItem> pItem(m_handler->createItem(dirUrl));

    StubExt st;
    bool called = false;
    st.set_lamda(&DFileService::renameFile, [&](){called = true; return true;});
    m_handler->rename(pItem.data(), "test_new_dir_name");
    EXPECT_TRUE(called);
}
