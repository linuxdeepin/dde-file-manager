#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "controllers/recentcontroller.cpp"
#undef private
#include "testhelper.h"
#include "dabstractfilewatcher.h"
#include "stubext.h"
#include "stub.h"
#include <dfmevent.h>

#include <QTimer>
#include <QSignalSpy>

using namespace stub_ext;
DFM_USE_NAMESPACE

namespace  {
class TestRecentController : public testing::Test
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


TEST_F(TestRecentController, openFileLocation)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, tmpFileUrl);
        EXPECT_TRUE(m_controller.openFileLocation(event));
        QProcess::execute("killall dde-file-manager");
    });
}

TEST_F(TestRecentController, test_openFile)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, tmpFileUrl);
        EXPECT_TRUE(m_controller.openFile(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecentController, test_openFiles)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() <<  DUrl::fromRecentFile("/"));
        StubExt st;
        st.set_lamda(&FileUtils::openFiles, []() { return true; });
        EXPECT_TRUE(m_controller.openFiles(event));
    });
}

TEST_F(TestRecentController, test_openFileByApp)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", tmpFileUrl);
        EXPECT_TRUE(m_controller.openFileByApp(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecentController, test_openFilesByApp)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << tmpFileUrl << tmpFileUrl2);
        EXPECT_TRUE(m_controller.openFilesByApp(event));
        QProcess::execute("killall deepin-editor");
    });
}

TEST_F(TestRecentController, test_writeFilesToClipboard)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << tmpFileUrl << tmpFileUrl2);
        EXPECT_TRUE(m_controller.writeFilesToClipboard(event));
    });
}

TEST_F(TestRecentController, test_moveToTrash)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << tmpFileUrl, true);
        DUrlList resList = m_controller.moveToTrash(event);
        EXPECT_TRUE(resList.isEmpty());
    });
}

TEST_F(TestRecentController, test_deleteFiles)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << tmpFileUrl);
        EXPECT_TRUE(m_controller.deleteFiles(event));
    });
}

TEST_F(TestRecentController, test_renameFile)
{
    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    DUrl to = DUrl::fromRecentFile(temp);
    auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, tmpFileUrl, to);
    tmpFileUrl = temp;
    bool (*stub_renameFile)(const QObject *, const DUrl &, const DUrl &, const bool)
    = [](const QObject *, const DUrl &, const DUrl &, const bool) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, renameFile), stub_renameFile);

    EXPECT_TRUE(m_controller.renameFile(event));
}

TEST_F(TestRecentController, test_setFileTags)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.setFileTags(event));
    });
}

TEST_F(TestRecentController,  test_removeTagsOfFile)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.setFileTags(event));
        auto removeEvent = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.removeTagsOfFile(removeEvent));
    });
}

TEST_F(TestRecentController, test_getTagsThroughFiles)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        m_controller.setFileTags(event);
        auto getTagsEvent = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({tmpFileUrl}));
        EXPECT_TRUE(!m_controller.getTagsThroughFiles(getTagsEvent).isEmpty());
        auto removeEvent = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
        EXPECT_TRUE(m_controller.removeTagsOfFile(removeEvent));
    });
}

TEST_F(TestRecentController, test_createSymlink)
{
    TestHelper::runInLoop([ = ] {
        QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
        auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, tmpFileUrl, DUrl::fromLocalFile(symlinkPath));
        EXPECT_TRUE(m_controller.createSymlink(event));
        TestHelper::deleteTmpFile(symlinkPath);
    });
}


TEST_F(TestRecentController, test_createFileInfo)
{
    TestHelper::runInLoop([ = ] {
        DUrl fileUrl = DUrl::fromRecentFile("/");
        const QSharedPointer<DFMCreateFileInfoEvent> &&event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl);
        const auto &&pt = m_controller.createFileInfo(event);
        EXPECT_NE(nullptr, pt.data());
    });
}

TEST_F(TestRecentController, createDirIterator)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, tmpDirUrl, QStringList(), QDir::AllEntries);
        auto iter = m_controller.createDirIterator(event);
        if (iter)
        {
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

TEST_F(TestRecentController, tst_createFileWatcher)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        auto ret = m_controller.createFileWatcher(event);
        ret->startWatcher();
        ret->stopWatcher();
        EXPECT_NE(nullptr, ret);
        delete ret;
        ret = nullptr;
    });
}

TEST_F(TestRecentController, tst_onFileModified)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller.createFileWatcher(event));
        QSignalSpy spy(watcher, SIGNAL(fileModified(const DUrl &)));
        DUrl url = DUrl::fromRecentFile("/");
        watcher->onFileModified(url);
        EXPECT_EQ(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst(); // take the first signal
        DUrl result = qvariant_cast<DUrl>(arguments.at(0));
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isRecentFile());
        delete watcher;
        watcher = nullptr;
    }, 1500);
}

TEST_F(TestRecentController, tst_onFileDeleted)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller.createFileWatcher(event));
        QSignalSpy spy(watcher, SIGNAL(fileDeleted(const DUrl &)));
        DUrl url = DUrl::fromRecentFile("/");
        watcher->onFileDeleted(url);
        EXPECT_EQ(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst(); // take the first signal
        DUrl result = qvariant_cast<DUrl>(arguments.at(0));
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isRecentFile());
        delete watcher;
        watcher = nullptr;
    }, 1500);
}

TEST_F(TestRecentController, tst_onFileAttributeChanged)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller.createFileWatcher(event));
        QSignalSpy spy(watcher, SIGNAL(fileAttributeChanged(const DUrl &)));
        DUrl url = DUrl::fromRecentFile("/");
        watcher->onFileAttributeChanged(url);
        EXPECT_EQ(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst(); // take the first signal
        DUrl result = qvariant_cast<DUrl>(arguments.at(0));
        EXPECT_TRUE(result.isValid());
        EXPECT_TRUE(result.isRecentFile());
        delete watcher;
        watcher = nullptr;
    }, 1500);
}

TEST_F(TestRecentController, tst_setEnabledSubfileWatcher_noRecentFile)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller.createFileWatcher(event));
        DUrl url = DUrl::fromSMBFile("/");
        const auto originUrlToWatcherMap = watcher->d_func()->urlToWatcherMap;
        watcher->setEnabledSubfileWatcher(url);
        const auto urlToWatcherMap = watcher->d_func()->urlToWatcherMap;
        EXPECT_EQ(originUrlToWatcherMap.size(), urlToWatcherMap.size());
        delete watcher;
        watcher = nullptr;
    }, 1500);
}

TEST_F(TestRecentController, tst_setEnabledSubfileWatcher)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromRecentFile("/"));
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller.createFileWatcher(event));
        QString tmpDir = TestHelper::createTmpDir();
        DUrl tmpUrl = DUrl::fromLocalFile(tmpDir);
        tmpUrl.setScheme(RECENT_SCHEME);
        watcher->setEnabledSubfileWatcher(tmpUrl);
        const auto urlToWatcherMap = watcher->d_func()->urlToWatcherMap;
        EXPECT_TRUE(urlToWatcherMap.contains(tmpUrl));
        delete watcher;
        watcher = nullptr;
    }, 1500);
}

TEST_F(TestRecentController, tst_compressFiles)
{
    TestHelper::runInLoop([ = ] {
        DUrlList urlList;
        urlList << DUrl::fromRecentFile("/");
        auto event = dMakeEventPointer<DFMCompressEvent>(nullptr, urlList);
        StubExt st;
        st.set_lamda(&DFileService::compressFiles, []() { return true; });
        EXPECT_TRUE(m_controller.compressFiles(event));
    }, 1500);
}

TEST_F(TestRecentController, tst_decompressFile)
{
    TestHelper::runInLoop([ = ] {
        DUrlList urlList;
        urlList << DUrl::fromRecentFile("/");
        auto event = dMakeEventPointer<DFMDecompressEvent>(nullptr, urlList);
        StubExt st;
        st.set_lamda(&DFileService::decompressFile, []()
        {
            return true;
        });
        EXPECT_TRUE(m_controller.decompressFile(event));
    }, 1500);
}
}

