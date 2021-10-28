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
static RecentController *m_controller;
class TestRecentController : public testing::Test
{
public:

    static void SetUpTestCase() {
        m_controller = new RecentController;
    }
    static void TearDownTestCase() {
        if (m_controller)
            m_controller->deleteLater();
    }
    void SetUp() override
    {
        std::cout << "start TestRecentController" << std::endl;
        QString tempFilePath = TestHelper::createTmpFile(".txt");
        QString tmpDirPath = TestHelper::createTmpDir();
        tmpFileUrl = DUrl::fromUserShareFile(tempFilePath);
        tmpDirUrl = DUrl::fromUserShareFile(tmpDirPath);
        tempFilePath = TestHelper::createTmpFile(".txt");
        tmpFileUrl2 = DUrl::fromUserShareFile(tempFilePath);
    }

    void TearDown() override
    {
        std::cout << "end TestRecentController" << std::endl;
        QStringList tmpPaths;
        tmpPaths << tmpFileUrl.path() << tmpDirUrl.path() << tmpFileUrl2.path();
        TestHelper::deleteTmpFiles(tmpPaths);
    }

public:
    DUrl tmpFileUrl;
    DUrl tmpFileUrl2;
    DUrl tmpDirUrl;
};

#ifndef __arm__
TEST_F(TestRecentController, openFileLocation)
{
    TestHelper::runInLoop([ = ] {});
    auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, tmpFileUrl);
    bool (*stub_openFileLocation)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };

    Stub stub;
    stub.set(ADDR(DFileService, openFileLocation), stub_openFileLocation);

    EXPECT_TRUE(m_controller->openFileLocation(event));

}
#endif

TEST_F(TestRecentController, test_openFile)
{
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, tmpFileUrl);

    bool (*stub_openFile)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };

    Stub stub;
    stub.set(ADDR(DFileService, openFile), stub_openFile);

    EXPECT_TRUE(m_controller->openFile(event));
}

TEST_F(TestRecentController, test_openFiles)
{
    auto event = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() <<  DUrl::fromRecentFile("/"));
    StubExt st;
    st.set_lamda(&FileUtils::openFiles, []() { return true; });
    EXPECT_TRUE(m_controller->openFiles(event));
}

TEST_F(TestRecentController, test_openFileByApp)
{
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop",tmpFileUrl);
    StubExt st;
    st.set_lamda(&DFileService::openFileByApp, []() { return true; });
    EXPECT_TRUE(m_controller->openFileByApp(event));
}

TEST_F(TestRecentController, test_openFilesByApp)
{
    auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << tmpFileUrl << tmpFileUrl2);
    StubExt st;
    st.set_lamda(&DFileService::openFilesByApp, []() { return true; });
    EXPECT_TRUE(m_controller->openFilesByApp(event));
}

TEST_F(TestRecentController, test_writeFilesToClipboard)
{
    auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << tmpFileUrl << tmpFileUrl2);
    StubExt st;
    st.set_lamda(&DFileService::writeFilesToClipboard, []() { return true; });
    EXPECT_TRUE(m_controller->writeFilesToClipboard(event));
}

TEST_F(TestRecentController, test_moveToTrash)
{
    auto event = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << tmpFileUrl, true);
    StubExt st;
    st.set_lamda(VADDR(RecentController,deleteFiles), []() { return true; });
    EXPECT_TRUE(m_controller->moveToTrash(event).isEmpty());
}

TEST_F(TestRecentController, test_deleteFiles)
{
    auto event = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << tmpFileUrl);
    StubExt st;
    st.set_lamda(&DRecentManager::removeItems, []() { return; });
    st.set_lamda(VADDR(DDialog,exec), [](){return 1;});
    EXPECT_TRUE(m_controller->deleteFiles(event));
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

    EXPECT_TRUE(m_controller->renameFile(event));
}

TEST_F(TestRecentController, test_setFileTags)
{

    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
    StubExt st;
    st.set_lamda(&DFileService::setFileTags, []() { return true; });
    EXPECT_TRUE(m_controller->setFileTags(event));
}

TEST_F(TestRecentController,  test_removeTagsOfFile)
{

    auto removeEvent = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, tmpFileUrl, QList<QString>({"红色"}));
    StubExt st;
    st.set_lamda(&DFileService::removeTagsOfFile, []() { return true; });
    EXPECT_TRUE(m_controller->removeTagsOfFile(removeEvent));

}

TEST_F(TestRecentController, test_getTagsThroughFiles)
{

    auto event = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({tmpFileUrl}));
    QList<QString> (*stub_getTagsThroughFiles)(const QObject *, const QList<DUrl> &, const bool) = [](const QObject *, const QList<DUrl> &, const bool) {
        return QList<QString>();
    };
    Stub stub;
    stub.set(ADDR(DFileService, getTagsThroughFiles), stub_getTagsThroughFiles);

    EXPECT_TRUE(m_controller->getTagsThroughFiles(event).isEmpty());
}

TEST_F(TestRecentController, test_createSymlink)
{

    QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, tmpFileUrl, DUrl::fromLocalFile(symlinkPath));
    bool (*stub_createSymlink)(const QObject *, const DUrl &, const DUrl &, bool)
    = [](const QObject *, const DUrl &, const DUrl &, bool) {
        return true;
    };
    Stub stub;
    stub.set((bool(DFileService::*)(const QObject *, const DUrl &, const DUrl &, bool) const)ADDR(DFileService, createSymlink), stub_createSymlink);

    EXPECT_TRUE(m_controller->createSymlink(event));
}

#ifndef __arm__
TEST_F(TestRecentController, test_createFileInfo)
{
    TestHelper::runInLoop([ = ] {
        DUrl fileUrl = DUrl::fromRecentFile("/");
        const QSharedPointer<DFMCreateFileInfoEvent> &&event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl);
        const auto &&pt = m_controller->createFileInfo(event);
        EXPECT_NE(nullptr, pt.data());
    });
}

TEST_F(TestRecentController, createDirIterator)
{
    TestHelper::runInLoop([ = ] {
        auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, tmpDirUrl, QStringList(), QDir::AllEntries);
        auto iter = m_controller->createDirIterator(event);
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
        auto ret = m_controller->createFileWatcher(event);
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
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller->createFileWatcher(event));
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
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller->createFileWatcher(event));
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
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller->createFileWatcher(event));
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
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller->createFileWatcher(event));
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
        RecentFileWatcher *watcher = static_cast<RecentFileWatcher *>(m_controller->createFileWatcher(event));
        QString tmpDir = TestHelper::createTmpDir();
        DUrl tmpUrl = DUrl::fromLocalFile(tmpDir);
        tmpUrl.setScheme(RECENT_SCHEME);
        watcher->setEnabledSubfileWatcher(tmpUrl);
        const auto urlToWatcherMap = watcher->d_func()->urlToWatcherMap;
        EXPECT_TRUE(urlToWatcherMap.contains(tmpUrl));
        delete watcher;
        watcher = nullptr;
        TestHelper::deleteTmpFile(tmpDir);
    }, 1500);
}
#endif

TEST_F(TestRecentController, tst_compressFiles)
{
    DUrlList urlList;
    urlList << DUrl::fromRecentFile("/");
    auto event = dMakeEventPointer<DFMCompressEvent>(nullptr, urlList);
    StubExt st;
    st.set_lamda(&DFileService::compressFiles, []() { return true; });
    EXPECT_TRUE(m_controller->compressFiles(event));
}

TEST_F(TestRecentController, tst_decompressFile)
{
    DUrlList urlList;
    urlList << DUrl::fromRecentFile("/");
    auto event = dMakeEventPointer<DFMDecompressEvent>(nullptr, urlList);
    StubExt st;
    st.set_lamda(&DFileService::decompressFile, []() { return true; });
    EXPECT_TRUE(m_controller->decompressFile(event));
}
}

