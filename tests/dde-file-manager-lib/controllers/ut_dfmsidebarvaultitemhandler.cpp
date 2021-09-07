#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <interfaces/dfmsidebaritem.h>
#include <views/dfmsidebar.h>
#include <views/dfilemanagerwindow.h>
#include <DDialog>
#include "vault/vaultlockmanager.h"
#include "interfaces/dfilemenumanager.h"
#include "stub-ext/stubext.h"

#include "stub.h"

#define private public
#include "controllers/dfmsidebarvaultitemhandler.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDFMSideBarVaultItemHandler : public testing::Test
    {
    public:
        QSharedPointer<DFMSideBarVaultItemHandler> m_controller;

        static void SetUpTestCase() {
            stub_ext::StubExt stu;
            stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});
        }

        virtual void SetUp() override
        {
            m_controller = QSharedPointer<DFMSideBarVaultItemHandler>(new DFMSideBarVaultItemHandler());
            std::cout << "start TestDFMSideBarVaultItemHandler" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMSideBarVaultItemHandler" << std::endl;
        }
    };
}


TEST_F(TestDFMSideBarVaultItemHandler, tst_contextMenu)
{
    DFMSideBarItem item;
    Stub stub;
    void (*ut_openNewTab)() = [](){};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;

    EXPECT_NE(nullptr, m_controller->contextMenu(window.getLeftSideBar(), &item));

    window.clearActions();
}

TEST_F(TestDFMSideBarVaultItemHandler, tst_lockNow)
{
    Stub stub;
    void (*ut_openNewTab)() = [](){};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;

    m_controller->lockNow(&window);
    window.clearActions();
}

TEST_F(TestDFMSideBarVaultItemHandler, tst_autoLock)
{
    bool (*st_autoLock)(VaultLockManager::AutoLockState) = [](VaultLockManager::AutoLockState){
        // do nothing.
        return  true;
    };
    Stub stub;
    stub.set(ADDR(VaultLockManager, autoLock), st_autoLock);

    m_controller->autoLock(0);
}

TEST_F(TestDFMSideBarVaultItemHandler, tst_showView)
{
    Stub stub;
    void (*ut_openNewTab)() = [](){};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;

    bool (*st_cd)(const DUrl &) = [](const DUrl &){
        return true;
    };
    stub.set(ADDR(DFileManagerWindow, cd), st_cd);

    m_controller->showView(&window, "");

    window.clearActions();
}
