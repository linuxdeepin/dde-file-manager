#include "dialogs/propertydialog.h"

#include <gtest/gtest.h>
#include <QWidget>

namespace  {
    class TestDFMRoundBackground : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pWidget = new QWidget();
            m_pTester = new DFMRoundBackground(m_pWidget, 20);
            std::cout << "start TestDFMRoundBackground";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            if(m_pWidget){
                delete m_pWidget;
                m_pWidget = nullptr;
            }
            std::cout << "end TestDFMRoundBackground";
        }
    public:
        DFMRoundBackground  *m_pTester;
        QWidget *m_pWidget;
    };
}

TEST_F(TestDFMRoundBackground, testInit)
{

}

TEST_F(TestDFMRoundBackground, testEventFilter)
{

}

namespace  {
    class TestNameTextEdit : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new NameTextEdit();
            std::cout << "start TestNameTextEdit";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestNameTextEdit";
        }
    public:
        NameTextEdit    *m_pTester;
    };
}

TEST_F(TestNameTextEdit, testInit)
{

}

TEST_F(TestNameTextEdit, testSetOrGerIsCanceled)
{
    bool isCanceled = true;
    m_pTester->setIsCanceled(isCanceled);
    bool result = m_pTester->isCanceled();
    EXPECT_TRUE(result);
}

TEST_F(TestNameTextEdit, testSetPlainText)
{
    QString str("unit test");
    m_pTester->setPlainText(str);
}

namespace  {
    class TestGroupTitleLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new GroupTitleLabel();
            std::cout << "start TestGroupTitleLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestGroupTitleLabel";
        }
    public:
        GroupTitleLabel *m_pTester;
    };
}

TEST_F(TestGroupTitleLabel, testInit)
{

}

namespace  {
    class TestSectionKeyLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new SectionKeyLabel();
            std::cout << "start TestSectionKeyLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestSectionKeyLabel";
        }
    public:
        SectionKeyLabel *m_pTester;
    };
}

TEST_F(TestSectionKeyLabel, testInit)
{

}

namespace  {
    class TestSectionValueLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new SectionValueLabel();
            std::cout << "start TestSectionValueLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestSectionValueLabel";
        }
    public:
        SectionValueLabel   *m_pTester;
    };
}

TEST_F(TestSectionValueLabel, testInit)
{

}

namespace  {
    class TestLinkSectionValueLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new LinkSectionValueLabel();
            std::cout << "start TestLinkSectionValueLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestLinkSectionValueLabel";
        }
    public:
        LinkSectionValueLabel   *m_pTester;
    };
}

TEST_F(TestLinkSectionValueLabel, testInit)
{

}

TEST_F(TestLinkSectionValueLabel, testSetOrGetLinkTargetUrl)
{
    DUrl url("file:///home");
    const char *strSrc = url.toString().toStdString().c_str();
    m_pTester->setLinkTargetUrl(url);
    DUrl result = m_pTester->linkTargetUrl();
    EXPECT_STREQ(strSrc, result.toString().toStdString().c_str());
}

namespace  {
    class TestPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pWidget = new QWidget();
            DFMEvent event(DFMEvent::OpenFile, m_pWidget);
            DUrl url("file:///test3");
            m_pTester = new PropertyDialog(event, url);
            std::cout << "start TestPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            if(m_pTester){
                delete m_pWidget;
                m_pWidget = nullptr;
            }
            std::cout << "end TestPropertyDialog";
        }
    public:
        PropertyDialog  *m_pTester;
        QWidget *m_pWidget;
    };
}

//TEST_F(TestPropertyDialog, testInit)
//{
//    m_pTester->show();
//}

//TEST_F(TestPropertyDialog, testStartComputerFolderSize)
//{
//    DUrl url("file::///test3");
//    m_pTester->startComputerFolderSize(url);
//}

//TEST_F(TestPropertyDialog, testToggleFileExecutable)
//{
//    m_pTester->toggleFileExecutable(false);
//}















