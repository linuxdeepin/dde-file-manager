#include <gtest/gtest.h>

#define private public
#define protected public
#include "plugins/dfmadditionalmenu_p.h"
#include "plugins/dfmadditionalmenu.h"
#include "plugins/dfmadditionalmenu.cpp"
#include "stub.h"
#include "../stub-ext/stubext.h"

DFM_USE_NAMESPACE

namespace  {
    class DFMAdditionalMenuTest : public testing::Test
    {
    public:
        void SetUp() override {
            p_menu = new DFMAdditionalMenu();
        }
        void TearDown() override {
            delete p_menu;
            p_menu = nullptr;
        }

        DFMAdditionalMenu   *p_menu = nullptr;
    };
}

TEST_F(DFMAdditionalMenuTest, test_appendParentMineType)
{
    ASSERT_NE(p_menu, nullptr);

    QStringList files;
    p_menu->appendParentMineType(files, files);

    files << ".txt" ;
    p_menu->appendParentMineType(files, files);
}

TEST_F(DFMAdditionalMenuTest, testActions_ZeroSize)
{
    ASSERT_NE(p_menu, nullptr);

    stub_ext::StubExt st;
    bool called = false;
    st.set_lamda(ADDR(DFMAdditionalMenuPrivate, emptyAreaActoins), [&called]()->QList<QAction *>{called = true;return QList<QAction*>();});

    QStringList files;
    QList<QAction *> result = p_menu->actions(files);
    EXPECT_TRUE(called);
}

TEST_F(DFMAdditionalMenuTest, testActions_OneSize)
{
    ASSERT_NE(p_menu, nullptr);

    QStringList files;
    files << "/home";
    QList<QAction *> result = p_menu->actions(files);
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(DFMAdditionalMenuTest, testActions_TwoSize)
{
    ASSERT_NE(p_menu, nullptr);

    QStringList files;
    files << "file:///test1" << "file:///test2";
    QList<QAction *> result = p_menu->actions(files);
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(DFMAdditionalMenuTest, load_desktop_file)
{
    ASSERT_NE(p_menu, nullptr);

    p_menu->loadDesktopFile();
    EXPECT_NE(p_menu->d_func()->menuActionHolder, nullptr);

    Stub stub;

    bool (*ut_contains)() = [](){return false;};
    stub.set(ADDR(XdgDesktopFile, contains), ut_contains);
    p_menu->loadDesktopFile();
    stub.reset(ADDR(XdgDesktopFile, contains));

    QStringList (*ut_actions)() = [](){return QStringList()<<"new"<<"del";};
    stub.set(ADDR(XdgDesktopFile, actions), ut_actions);
    p_menu->loadDesktopFile();
    stub.reset(ADDR(XdgDesktopFile, actions));
}

TEST_F(DFMAdditionalMenuTest, test_emptyAreaActoins)
{
    ASSERT_NE(p_menu, nullptr);

    QMap<QString, QList<QAction *> > &temp = p_menu->d_func()->actionListByType;
    if (!temp.isEmpty() && !temp.contains(QString("EmptyArea"))) {
        auto value = temp.first();
        temp.insert(QString("EmptyArea"), value);
    }

    p_menu->d_func()->emptyAreaActoins(QString("/home/testa"), false);
}

TEST_F(DFMAdditionalMenuTest, test_single)
{
    ASSERT_NE(p_menu, nullptr);

    QMap<QString, QList<QAction *> > &temp = p_menu->d_func()->actionListByType;
    if (!temp.isEmpty() && !temp.contains(QString("EmptyArea"))) {
        auto value = temp.first();
        temp.insert(QString("EmptyArea"), value);
    }

    p_menu->d_func()->emptyAreaActoins(QString("/home/testa"), false);
}
