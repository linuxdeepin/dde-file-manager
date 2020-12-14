#include <gtest/gtest.h>
#include <QLabel>

#define private public
#include "dialogs/trashpropertydialog.h"

namespace  {
    class TestTrashPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
            std::cout << "start TestTrashPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestTrashPropertyDialog";
        }
    public:
        TrashPropertyDialog *m_pTester;
    };
}

TEST_F(TestTrashPropertyDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestTrashPropertyDialog, testUpdateFolderSize)
{
    m_pTester->updateFolderSize(1);
    QString str = m_pTester->m_sizeLabel->text();
    EXPECT_TRUE(str == "1 B");
}
