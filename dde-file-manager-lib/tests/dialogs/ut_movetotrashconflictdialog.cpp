#include "dialogs/movetotrashconflictdialog.h"
#include "stub.h"
#include "dfmglobal.h"

#include <gtest/gtest.h>

namespace  {
    class TestMoveToTrashConflictDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new MoveToTrashConflictDialog();
            std::cout << "start TestMoveToTrashConflictDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestMoveToTrashConflictDialog";
        }
    public:
        MoveToTrashConflictDialog   *m_pTester;
    };
}

TEST_F(TestMoveToTrashConflictDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestMoveToTrashConflictDialog, testInit2)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    DUrlList lst;
    lst << DUrl("file:///jerry");
    MoveToTrashConflictDialog dlg(nullptr, lst);
    QString str = dlg.title();
    EXPECT_TRUE(str == "This file is too big for the trash");
}

