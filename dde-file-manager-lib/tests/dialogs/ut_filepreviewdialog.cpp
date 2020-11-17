#include "dialogs/filepreviewdialog.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

namespace  {
    class TestUnknowFilePreView : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new UnknowFilePreview();
            std::cout << "start TestUnknowFilePreView";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestUnknowFilePreView";
        }
    public:
        UnknowFilePreview   *m_pTester;
    };
}

TEST_F(TestUnknowFilePreView, testSetFileUrl)
{
    DUrl url("file:///test1");
    bool result = m_pTester->setFileUrl(url);
    EXPECT_TRUE(result);
}

TEST_F(TestUnknowFilePreView, testFileUrl)
{
    DUrl url("file:///test1");
    QString strUrl1 = url.toString();
    m_pTester->setFileUrl(url);
    DUrl result = m_pTester->fileUrl();
    QString strUrl2 = result.toString();
    EXPECT_STREQ(strUrl1.toStdString().c_str(),
                 strUrl2.toStdString().c_str());
}

TEST_F(TestUnknowFilePreView, testContentWidget)
{
    QWidget *result = nullptr;
    result = m_pTester->contentWidget();
    EXPECT_TRUE(result != nullptr);
}

TEST_F(TestUnknowFilePreView, testUpdateFolderSize)
{
//    qint64 size = 1024;
//    m_pTester->updateFolderSize(size);
}

namespace  {
    class TestFilePreviewDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            DUrlList previewUrllist;
            previewUrllist << DUrl("file://~/test1");
            m_pTester = new FilePreviewDialog(previewUrllist, nullptr);
            std::cout << "start TestFilePreviewDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestFilePreviewDialog";
        }
    public:
        FilePreviewDialog   *m_pTester;
    };
}

TEST_F(TestFilePreviewDialog, testInit)
{
    m_pTester->show();
}

TEST_F(TestFilePreviewDialog, testUpdatePreviewList)
{
    DUrlList previewUrllist;
    previewUrllist << DUrl("file://~/test2");
    m_pTester->updatePreviewList(previewUrllist);
}

TEST_F(TestFilePreviewDialog, testSetEntryUrlList)
{
    DUrlList entryUrlList;
    entryUrlList.clear();
    m_pTester->setEntryUrlList(entryUrlList);
}

TEST_F(TestFilePreviewDialog, testDone)
{
    int r = 1;
    m_pTester->done(r);
}

TEST_F(TestFilePreviewDialog, testPlayCurrentPreviewFile)
{
    m_pTester->playCurrentPreviewFile();
}
