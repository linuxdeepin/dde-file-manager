#include "dialogs/dmultifilepropertydialog.h"
#include "stub.h"

#include <gtest/gtest.h>


namespace  {
    class TestDMultiFilePropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            QList<DUrl> urlList;
            urlList << DUrl("file:///test1") << DUrl("file:///test2");

            void(*stu_startComputingFolderSize)() = [](){};
            Stub stu;
            stu.set(ADDR(DMultiFilePropertyDialog, startComputingFolderSize), stu_startComputingFolderSize);

            m_pTester = new DMultiFilePropertyDialog(urlList);
            std::cout << "start TestDMultiFilePropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDMultiFilePropertyDialog";
        }
    public:
        DMultiFilePropertyDialog *m_pTester;
    };
}

TEST_F(TestDMultiFilePropertyDialog, testInit)
{
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestDMultiFilePropertyDialog, testStartComputingFolderSize)
{
//    m_pTester->startComputingFolderSize();
}

TEST_F(TestDMultiFilePropertyDialog, testGetRandomNumber)
{
    std::pair<std::size_t, std::size_t> scope;
    scope.first = std::size_t(100);
    scope.second = std::size_t(100);
    std::size_t result = DMultiFilePropertyDialog::getRandomNumber(scope);
    EXPECT_EQ(result, 100);
}
