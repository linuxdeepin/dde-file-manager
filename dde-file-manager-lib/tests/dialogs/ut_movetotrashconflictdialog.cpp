#include "dialogs/movetotrashconflictdialog.h"

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
//    m_pTester->show();
}
