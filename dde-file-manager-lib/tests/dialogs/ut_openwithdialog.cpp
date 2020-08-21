#include "dialogs/openwithdialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestOpenWithDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            DUrl url("file:///test3");
            m_pTester = new OpenWithDialog(url);
            std::cout << "start TestOpenWithDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestOpenWithDialog";
        }
    public:
        OpenWithDialog  *m_pTester;
    };
}

TEST_F(TestOpenWithDialog, testInit)
{
//    m_pTester->show();
}

TEST_F(TestOpenWithDialog, testInit2)
{
    QList<DUrl> urllist;
    urllist << DUrl("file:///test1") << DUrl("file:///test2");
    OpenWithDialog dlg(urllist);
    dlg.show();
}

TEST_F(TestOpenWithDialog, testOpenFileByApp)
{
    m_pTester->openFileByApp();
}
