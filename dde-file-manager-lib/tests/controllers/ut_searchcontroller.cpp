#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include "dfmevent.h"

#define private public
#include "controllers/searchcontroller.cpp"

namespace  {
class TestSearchController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestSearchController" << std::endl;
        controller = new SearchController;
        filePath_1 =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/1.txt";
        filePath_2 =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/2.txt";
        dirPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/hello";
        QProcess::execute("touch " + filePath_1 + " " + filePath_2);
        QProcess::execute("mkdir " + dirPath);
        fileUrl_1.setScheme(SEARCH_SCHEME);
        fileUrl_1.setQuery("url=file:///tmp&keyword=hello");
        fileUrl_1.setSearchedFileUrl(DUrl::fromLocalFile(filePath_1));

        fileUrl_2.setScheme(SEARCH_SCHEME);
        fileUrl_2.setQuery("url=file:///tmp&keyword=hello");
        fileUrl_2.setSearchedFileUrl(DUrl::fromLocalFile(filePath_2));

        dirUrl.setScheme(SEARCH_SCHEME);
        dirUrl.setQuery("url=file:///tmp&keyword=hello");
        dirUrl.setSearchedFileUrl(DUrl::fromLocalFile(dirPath));
    }

    void TearDown() override
    {
        std::cout << "end TestSearchController" << std::endl;
        QProcess::execute("rm -rf " + filePath_1 + " " + filePath_2 + " " + dirPath);
        delete controller;
        controller = nullptr;
    }

public:
    SearchController *controller;
    QString filePath_1;
    QString filePath_2;
    QString dirPath;
    DUrl fileUrl_1;
    DUrl fileUrl_2;
    DUrl dirUrl;
};

TEST_F(TestSearchController, createFileInfo)
{
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl_1);
    EXPECT_TRUE(controller->createFileInfo(event) != nullptr);
}

TEST_F(TestSearchController, openFileLocation)
{
//    auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, fileUrl_1);
//    EXPECT_TRUE(controller->openFileLocation(event));
}

TEST_F(TestSearchController, openFile)
{
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, fileUrl_1);
    EXPECT_TRUE(controller->openFile(event));
}

TEST_F(TestSearchController, openFileByApp)
{
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", fileUrl_1);
    EXPECT_TRUE(controller->openFileByApp(event));
}

TEST_F(TestSearchController, openFilesByApp)
{
    auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << fileUrl_1 << fileUrl_2);
    EXPECT_TRUE(controller->openFilesByApp(event));
}

TEST_F(TestSearchController, writeFilesToClipboard)
{
    auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << fileUrl_1 << fileUrl_2);
    EXPECT_TRUE(controller->writeFilesToClipboard(event));
}

TEST_F(TestSearchController, moveToTrash)
{
//    auto event = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << fileUrl_1);
//    DUrlList resList = controller->moveToTrash(event);
//    EXPECT_TRUE(!resList.isEmpty());
}

TEST_F(TestSearchController, deleteFiles)
{
//    auto event = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << fileUrl_1);
//    EXPECT_TRUE(controller->deleteFiles(event));
}

TEST_F(TestSearchController, renameFile)
{
    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    DUrl to;
    to.setScheme(SEARCH_SCHEME);
    to.setSearchedFileUrl(DUrl::fromLocalFile(temp));
    auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, fileUrl_1, to);
    filePath_1 = temp;
    EXPECT_TRUE(controller->renameFile(event));
}

TEST_F(TestSearchController, setPermissions)
{
    auto event = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, fileUrl_1, (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadOwner));
    EXPECT_TRUE(controller->setPermissions(event));
}

TEST_F(TestSearchController, addToBookmark)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, dirUrl);
    // EXPECT_TRUE(!controller->addToBookmark(event));
}

TEST_F(TestSearchController, removeBookmark)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, dirUrl);
    // EXPECT_TRUE(!controller->removeBookmark(event));
}

TEST_F(TestSearchController, createSymlink)
{
    QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fileUrl_1, DUrl::fromLocalFile(symlinkPath));
    EXPECT_TRUE(controller->createSymlink(event));
    QProcess::execute("rm -f " + symlinkPath);
}

TEST_F(TestSearchController, shareFolder)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, dirUrl, "hello");
    // EXPECT_TRUE(controller->shareFolder(event));
}

TEST_F(TestSearchController, unShareFolder)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, dirUrl, "hello");
    // controller->shareFolder(event);

    // auto unShareFolderEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, dirUrl);
    // EXPECT_TRUE(controller->unShareFolder(unShareFolderEvent));
}

TEST_F(TestSearchController, openInTerminal)
{
    auto event = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, dirUrl);
    EXPECT_TRUE(controller->openInTerminal(event));
}

TEST_F(TestSearchController, createDirIterator)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, dirUrl, QStringList(), QDir::AllEntries);
    // auto iter = controller->createDirIterator(event);
    // if (iter) {
    //     iter->hasNext();
    //     iter->next();
    //     iter->fileUrl();
    //     iter->fileInfo();
    //     iter->fileName();
    //     iter->url();
    //     iter->close();
    // }
    // EXPECT_TRUE(iter != nullptr);

    // 处理消息队列，否则其余位置直[间]接调用会导致事件处理顺序问题而 crash
    // qApp->processEvents();
}

TEST_F(TestSearchController, createFileWatcher)
{
    DUrl url(SEARCH_SCHEME);
    url.setQuery("url=file:///tmp&keyword=hello");
    auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url);
    SearchFileWatcher *watcher = static_cast<SearchFileWatcher *>(controller->createFileWatcher(event));
    if (watcher) {
        watcher->addWatcher(fileUrl_1);
        watcher->removeWatcher(fileUrl_1);
        watcher->onFileModified(fileUrl_1);
        watcher->onFileDeleted(fileUrl_1);
        watcher->onFileMoved(fileUrl_1, fileUrl_2);
        watcher->onFileAttributeChanged(fileUrl_1);
    }
    EXPECT_TRUE(watcher != nullptr);
}

TEST_F(TestSearchController, setFileTags)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    EXPECT_TRUE(controller->setFileTags(event));
}

TEST_F(TestSearchController, removeTagsOfFile)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    controller->setFileTags(event);
    auto event1 = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    EXPECT_TRUE(controller->removeTagsOfFile(event1));
}

TEST_F(TestSearchController, getTagsThroughFiles)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    controller->setFileTags(event);
    auto event1 = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({fileUrl_1}));
    EXPECT_TRUE(!controller->getTagsThroughFiles(event1).isEmpty());
}
}
