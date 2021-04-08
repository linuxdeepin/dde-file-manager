#include <gtest/gtest.h>
#include <QTimer>
#include <QLabel>

#include "stub.h"
#include "app/define.h"
#include "dfileservices.h"
#include "interfaces/dfileinfo.h"
#include "io/dfilestatisticsjob.h"

#define private public
#include "dialogs/filepreviewdialog.h"
#include "plugins/dfmfilepreviewfactory.h"

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
    DFileStatisticsJob job;
    m_pTester->m_sizeWorker = &job;
    qint64 size = 1024;
    m_pTester->updateFolderSize(size);
    QString str = m_pTester->m_sizeLabel->text();
    EXPECT_NE("", str.toStdString().c_str());
    m_pTester->m_sizeWorker = nullptr;
}

TEST_F(TestUnknowFilePreView, testSetFileInfo)
{
    DUrl url("file:///test1");

    bool(*stub_isFile)(void *) = [](void *)->bool{
        return true;
    };
    typedef bool (*fptr)();
    Stub stu;
    stu.set((fptr)(&DFileInfo::isFile), stub_isFile);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){
        int a = 0;
    };
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);

    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, url);
    m_pTester->setFileInfo(info);
    EXPECT_EQ(m_pTester->m_iconLabel->text(), "");
}

TEST_F(TestUnknowFilePreView, testSetFileInfo2)
{
    DUrl url("file:///test1");

    bool(*stub_isFile)(void *) = [](void *)->bool{
        return true;
    };
    typedef bool (*fptr)();
    Stub stu;
    stu.set((fptr)(&DFileInfo::isDir), stub_isFile);

    void(*stu_start)(const DUrlList &) = [](const DUrlList &){
        int a = 0;
    };
    Stub stu2;
    stu2.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);


    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, url);
    m_pTester->setFileInfo(info);
    EXPECT_EQ(m_pTester->m_iconLabel->text(), "");
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
            if(m_pTester)
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
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestFilePreviewDialog, testUpdatePreviewList)
{
    DUrlList previewUrllist;
    previewUrllist << DUrl("file:///test2");
    m_pTester->updatePreviewList(previewUrllist);
    int count = m_pTester->m_fileList.count();
    EXPECT_EQ(count, 1);
}

TEST_F(TestFilePreviewDialog, testUpdatePreviewList2)
{
    DUrlList previewUrllist;
    previewUrllist << DUrl("file:///test2") << DUrl("file:///test2");
    m_pTester->updatePreviewList(previewUrllist);
    int count = m_pTester->m_fileList.count();
    EXPECT_EQ(count, 2);
}

TEST_F(TestFilePreviewDialog, testSetEntryUrlList)
{
    DUrlList entryUrlList;
    entryUrlList.clear();
    m_pTester->setEntryUrlList(entryUrlList);
    EXPECT_EQ(m_pTester->m_fileList.count(), 0);
}

TEST_F(TestFilePreviewDialog, testSetEntryUrlList2)
{
    m_pTester->m_fileList.push_back(DUrl("file:///test1"));
    DUrlList entryUrlList;
    entryUrlList << DUrl("file:///test1") << DUrl("file:///test2");
    m_pTester->setEntryUrlList(entryUrlList);
    EXPECT_EQ(m_pTester->m_entryUrlList.count(), 2);
}

TEST_F(TestFilePreviewDialog, testDone)
{
    int r = 1;

    UnknowFilePreview view;
    m_pTester->m_preview = &view;

    EXPECT_NO_FATAL_FAILURE(m_pTester->done(r));
}

TEST_F(TestFilePreviewDialog, testDone2)
{
    int r = 1;

    UnknowFilePreview* pview = new UnknowFilePreview();
    m_pTester->m_preview = pview;

    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    EXPECT_NO_FATAL_FAILURE(m_pTester->done(r));
}

TEST_F(TestFilePreviewDialog, testDoneCurrent)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->DoneCurrent());
}

TEST_F(TestFilePreviewDialog, testPlayCurrentPreviewFile)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->playCurrentPreviewFile());
}

TEST_F(TestFilePreviewDialog, testShowEvent)
{
    // 阻塞CI
    // QShowEvent event;
    // EXPECT_NO_FATAL_FAILURE(m_pTester->showEvent(&event));
}

TEST_F(TestFilePreviewDialog, testEventFilter)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(&obj, &event));
}

TEST_F(TestFilePreviewDialog, testEventFilter2)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(&obj, &event));
}

TEST_F(TestFilePreviewDialog, testEventFilter3)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(&obj, &event));
}

TEST_F(TestFilePreviewDialog, testEventFilter4)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F33, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(&obj, &event));
}

TEST_F(TestFilePreviewDialog, testUpdateTitle)
{
    m_pTester->updateTitle();
    EXPECT_NE(m_pTester->m_statusBar, nullptr);
}

TEST_F(TestFilePreviewDialog, testCloseEvent)
{
    // 阻塞CI
    // QCloseEvent event;

    // bool(*stub_isWayLand)() = []()->bool{
    //     return true;
    // };
    // Stub stu;
    // stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    // EXPECT_NO_FATAL_FAILURE(m_pTester->closeEvent(&event));
}

TEST_F(TestFilePreviewDialog, testResizeEvent)
{
    // 阻塞CI
    // QResizeEvent event(QSize(100,100), QSize(200, 200));
    // m_pTester->resizeEvent(&event);
    // QEventLoop loop;
    // QTimer::singleShot(100, nullptr, [&loop](){loop.exit();});
    // loop.exec();
    // EXPECT_NE(m_pTester->width(), 100);
}

TEST_F(TestFilePreviewDialog, testPreviousPage)
{
    m_pTester->m_currentPageIndex = 2;

    void(*stub_switchToPage)(int) = [](int){

    };
    Stub stu;
    stu.set(ADDR(FilePreviewDialog, switchToPage), stub_switchToPage);

    m_pTester->previousPage();
    EXPECT_EQ(m_pTester->m_firstEnterSwitchToPage, false);
}

TEST_F(TestFilePreviewDialog, testPreviousPage2)
{
    m_pTester->m_currentPageIndex = 2;
    m_pTester->m_playingVideo = true;
    m_pTester->previousPage();
    EXPECT_EQ(m_pTester->m_firstEnterSwitchToPage, true);
}

void stub_switchToPage(int){

};

TEST_F(TestFilePreviewDialog, testnextPage)
{
    m_pTester->m_currentPageIndex = -2;

    Stub stu;
    stu.set(ADDR(FilePreviewDialog, switchToPage), stub_switchToPage);

    m_pTester->nextPage();
    EXPECT_EQ(m_pTester->m_firstEnterSwitchToPage, false);
}

TEST_F(TestFilePreviewDialog, testnextPage2)
{
    m_pTester->m_currentPageIndex = 2;
    m_pTester->m_playingVideo = true;
    m_pTester->nextPage();
    EXPECT_EQ(m_pTester->m_firstEnterSwitchToPage, true);
}
