#include "dialogs/shareinfoframe.h"
#include "models/dfmrootfileinfo.h"

#include <gtest/gtest.h>

namespace  {
    class TestShareInfoFrame : public testing::Test
    {
    public:
        void SetUp() override
        {
            DAbstractFileInfoPointer fi(new DFMRootFileInfo(DUrl("file:///test1")));;
            m_pTester = new ShareInfoFrame(fi);
            std::cout << "start TestShareInfoFrame";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestShareInfoFrame";
        }
    public:
        ShareInfoFrame  *m_pTester;
    };
}

TEST_F(TestShareInfoFrame, testInit)
{
//    m_pTester->show();
}

TEST_F(TestShareInfoFrame, testSetFileinfo)
{
    DAbstractFileInfoPointer fileinfo(new DFMRootFileInfo(DUrl("file:///test1")));
    m_pTester->setFileinfo(fileinfo);
}

TEST_F(TestShareInfoFrame, testCheckShareName)
{
    EXPECT_TRUE(m_pTester->checkShareName());
}

TEST_F(TestShareInfoFrame, testHandleCheckBoxChanged)
{
    m_pTester->handleCheckBoxChanged(false);
}

TEST_F(TestShareInfoFrame, testHandleShareNameChanged)
{
    m_pTester->handleShareNameChanged();
}

TEST_F(TestShareInfoFrame, testHandlePermissionComboxChanged)
{
    m_pTester->handlePermissionComboxChanged(1);
}

TEST_F(TestShareInfoFrame, testHandleAnonymityComboxChanged)
{
    m_pTester->handleAnonymityComboxChanged(1);
}

TEST_F(TestShareInfoFrame, testHandShareInfoChanged)
{

}

TEST_F(TestShareInfoFrame, testDisactivateWidgets)
{
    m_pTester->disactivateWidgets();
}


























