#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "dfmstandardpaths.h"
#include <QProcess>
#include "tag/tagmanager.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfmsidebaritem.h"
#include "interfaces/dfilemenumanager.h"

#include <QMessageBox>

#include "stub.h"
#include "stub-ext/stubext.h"

#define private public
#define protected public

#include "controllers/dfmsidebartagitemhandler.h"

DFM_USE_NAMESPACE

using namespace testing;
namespace  {
    class TestDFMSideBarTagItemHandler : public Test
    {
    public:
        TestDFMSideBarTagItemHandler():Test()
        {
            m_handler = nullptr;
        }

        virtual void SetUp() override {
            m_handler = new DFMSideBarTagItemHandler();
        }

        virtual void TearDown() override {
            delete  m_handler;
        }

        DFMSideBarTagItemHandler *m_handler;
    };
}

TEST_F(TestDFMSideBarTagItemHandler, tst_createItem)
{ 
    QString testFile = DFMStandardPaths::location(DFMStandardPaths::PicturesPath) + "/utFile";

    QString cmdTouch = QString("touch ") + testFile;
    QString cmdRm = QString("rm ") + testFile;

    QProcess::execute(cmdTouch);

    QString (*st_getTagIconName)(void *, const QString &) = [](void *,const QString &){
        return QString();
    };

    Stub stub;
    stub.set((QString(TagManager::*)(const QString &)const)ADDR(TagManager, getTagIconName), st_getTagIconName);

    DUrl url(testFile);
    auto item = m_handler->createItem(url);
    EXPECT_NE(nullptr, item);
    if (item) {
        delete item;
        item = nullptr;
    }
}

TEST_F(TestDFMSideBarTagItemHandler, tst_contextMenu)
{
    return;
    Stub stub;
    static bool myCallOpen = false;
    void (*ut_openNewTab)() = [](){myCallOpen = true;};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    DFileManagerWindow window;
    EXPECT_TRUE(myCallOpen);

    const DFMSideBar *bar = window.getLeftSideBar();

    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
    ASSERT_NE(item, nullptr);

    typedef QMessageBox::StandardButton(*FunPtr)(
                QWidget *, const QString &, const QString &, QMessageBox::StandardButtons, QMessageBox::StandardButton);

    QMessageBox::StandardButton (*st_warning)(
                QWidget *, const QString &, const QString &, QMessageBox::StandardButtons, QMessageBox::StandardButton) =
            [](QWidget *, const QString &, const QString &, QMessageBox::StandardButtons, QMessageBox::StandardButton) {
        return  QMessageBox::NoButton;
    };
    stub.set(FunPtr(&QMessageBox::warning), st_warning);

    QMenu *menu = m_handler->contextMenu(bar, item);
    QList<QAction *> actions = menu->actions();
    for (auto action : actions) {
        action->trigger();
    }

    EXPECT_NE(menu, nullptr);

    delete item;
    delete menu;
}

TEST_F(TestDFMSideBarTagItemHandler, tst_rename)
{
    Stub stub;
    static bool myCallOpen = false;
    void (*ut_openNewTab)() = [](){myCallOpen = true;};
    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

    DFileManagerWindow window;
    EXPECT_TRUE(myCallOpen);

    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
    ASSERT_NE(item, nullptr);

    EXPECT_NO_FATAL_FAILURE(m_handler->rename(item, "newName"));

    window.clearActions();

    delete item;
}
