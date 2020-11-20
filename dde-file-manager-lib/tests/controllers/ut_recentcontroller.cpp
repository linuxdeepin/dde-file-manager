#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTimer>
#include <dfmevent.h>
#include "controllers/recentcontroller.h"
#include "testhelper.h"
#include "dabstractfilewatcher.h"

DFM_USE_NAMESPACE
namespace  {
    class TestRecntController : public testing::Test
    {
        void SetUp() override
        {
            std::cout << "start TestRecntController" << std::endl;
            QString tempFilePath = TestHelper::createTmpFile(".txt");
            QString tmpDirPath = TestHelper::createTmpDir();
            tmpFileUrl = DUrl::fromUserShareFile(tempFilePath);
            tmpDirUrl = DUrl::fromUserShareFile(tmpDirPath);
            tempFilePath = TestHelper::createTmpFile(".txt");
            tmpFileUrl2 = DUrl::fromUserShareFile(tempFilePath);
        }

        void TearDown() override
        {
            std::cout << "end TestRecntController" << std::endl;
            QStringList tmpPaths;
            tmpPaths << tmpFileUrl.path() << tmpDirUrl.path() << tmpFileUrl2.path();
            TestHelper::deleteTmpFiles(tmpPaths);
        }

    public:
        RecentController m_controller;
        DUrl tmpFileUrl;
        DUrl tmpFileUrl2;
        DUrl tmpDirUrl;
    };


TEST_F(TestRecntController, openFileLocation)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, tmpFileUrl);
        EXPECT_TRUE(m_controller.openFileLocation(event));
        QProcess::execute("killall dde-file-manager");
    });
}

TEST_F(TestRecntController, test_openFile)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, tmpFileUrl);
        EXPECT_TRUE(m_controller.openFile(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecntController, test_openFileByApp)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", tmpFileUrl);
        EXPECT_TRUE(m_controller.openFileByApp(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecntController, test_openFilesByApp)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << tmpFileUrl << tmpFileUrl2);
        EXPECT_TRUE(m_controller.openFilesByApp(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecntController, test_writeFilesToClipboard)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << tmpFileUrl << tmpFileUrl2);
        EXPECT_TRUE(m_controller.writeFilesToClipboard(event));
    });
}

TEST_F(TestRecntController, test_moveToTrash)
{
    TestHelper::runInLoop([=] {
        auto event = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << tmpFileUrl, true);
        DUrlList resList = m_controller.moveToTrash(event);
        EXPECT_TRUE(resList.isEmpty());
    });
}

TEST_F(TestRecntController, test_deleteFiles)
{
    TestHelper::runInLoop([=] {
        auto event = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << tmpFileUrl);
        EXPECT_TRUE(m_controller.deleteFiles(event));
    });
}

TEST_F(TestRecntController, test_renameFile)
{
    TestHelper::runInLoop([=]{
        QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
        DUrl to = DUrl::fromRecentFile(temp);
        auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, tmpFileUrl, to);
        tmpFileUrl = temp;
        EXPECT_TRUE(m_controller.renameFile(event));
    });
}

TEST_F(TestRecntController, test_setFileTags)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.setFileTags(event));
    });
}

TEST_F(TestRecntController,  test_removeTagsOfFile)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.setFileTags(event));
        auto removeEvent = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.removeTagsOfFile(removeEvent));
    });
}

TEST_F(TestRecntController, test_getTagsThroughFiles)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        m_controller.setFileTags(event);
        auto getTagsEvent = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({tmpFileUrl}));
        EXPECT_TRUE(!m_controller.getTagsThroughFiles(getTagsEvent).isEmpty());
        auto removeEvent = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.removeTagsOfFile(removeEvent));
    });
}

TEST_F(TestRecntController, test_createSymlink)
{
    TestHelper::runInLoop([=]{
        QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
        auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, tmpFileUrl, DUrl::fromLocalFile(symlinkPath));
        EXPECT_TRUE(m_controller.createSymlink(event));
        TestHelper::deleteTmpFile(symlinkPath);
    });
}


TEST_F(TestRecntController, test_createFileInfo)
{
    TestHelper::runInLoop([=]{
        DUrl fileUrl = DUrl::fromRecentFile("/");
        const QSharedPointer<DFMCreateFileInfoEvent> &&event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl);
        const auto &&pt = m_controller.createFileInfo(event);
        EXPECT_NE(nullptr, pt.data());
    });
}

TEST_F(TestRecntController, createDirIterator)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, tmpDirUrl, QStringList(), QDir::AllEntries);
        auto iter = m_controller.createDirIterator(event);
        if (iter) {
            iter->hasNext();
            iter->next();
            iter->fileUrl();
            iter->fileInfo();
            iter->fileName();
            iter->url();
            iter->close();
        }
        EXPECT_TRUE(iter != nullptr);
        // 处理消息队列，否则其余位置直[间]接调用会导致事件处理顺序问题而 crash
        qApp->processEvents();
    });

}

TEST_F(TestRecntController, tst_createFileWatcher)
{
    TestHelper::runInLoop([=]{
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        auto ret = m_controller.createFileWatcher(event);
        ret->startWatcher();
        ret->stopWatcher();
        EXPECT_NE(nullptr, ret);
    });
}
}

