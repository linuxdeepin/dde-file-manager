#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include "dfmevent.h"

#define private public
#include "tag/tagmanager.h"
#include "controllers/tagcontroller.cpp"

#define TAG_FILE_PATH_STR "test_tag_"
#define TAG_NAME_A "a"
#define TAG_NAME_B "b"
#define TAG_TXT_FILE "tag_test.txt";
#define NEW_TAG_TXT_FILE "new_tag_test.txt";

namespace  {
class TestTagController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTagController" << std::endl;

        m_pController = new TagController();
        m_pManager = new TagManager();

        tempDirPath_A = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + TAG_FILE_PATH_STR + TAG_NAME_A;
        tempDirPath_B = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + TAG_FILE_PATH_STR + TAG_NAME_B;
        tempTxtFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first()  + "/" + TAG_TXT_FILE;

        m_tagUrl.setScheme(TAG_SCHEME);
        m_tagUrl.setQuery("tagname=" + QString(TAG_NAME_A));
        m_tagUrl.setPath("/" + QString(TAG_NAME_A));

        m_tagFileUrl.setScheme(TAG_SCHEME);
        m_tagFileUrl.setTaggedFileUrl(tempTxtFilePath);
    }

    void TearDown() override
    {
        delete m_pManager;
        m_pManager = nullptr;

        delete m_pController;
        m_pController = nullptr;

        std::cout << "end TestTagController" << std::endl;
    }

public:
    TagController *m_pController = nullptr;
    TagManager *m_pManager = nullptr;
    QString tempDirPath_A;
    QString tempDirPath_B;
    QString tempTxtFilePath;
    DUrl m_tagUrl;
    DUrl m_tagFileUrl;
};

TEST_F(TestTagController, test_prepare)
{
    QProcess::execute("mkdir " + tempDirPath_A + " " + tempDirPath_B);
    QProcess::execute("touch " + tempTxtFilePath);

    ASSERT_NE(m_pManager, nullptr);
    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };
    m_pManager->makeFilesTags(tags, files);
}

TEST_F(TestTagController, can_create_file_info)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->createFileInfo(event) != nullptr);
}

TEST_F(TestTagController, can_getChildren)
{
    ASSERT_NE(m_pController, nullptr);

    QSharedPointer<DFMGetChildrensEvent> event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, m_tagUrl, QStringList(), QDir::AllEntries | QDir::Hidden);
    EXPECT_TRUE(!m_pController->getChildren(event).isEmpty());

    m_tagUrl.setPath("/");
    auto eventRoot = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, m_tagUrl, QStringList(), QDir::AllEntries | QDir::Hidden);
    EXPECT_TRUE(!m_pController->getChildren(eventRoot).isEmpty());
}

TEST_F(TestTagController, can_creatFileWatcher)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, m_tagUrl);
    TaggedFileWatcher *watcher = static_cast<TaggedFileWatcher *>(m_pController->createFileWatcher(event));
    if (watcher) {
        watcher->addWatcher(m_tagUrl);
        watcher->removeWatcher(m_tagUrl);
    }
    EXPECT_TRUE(m_pController->createFileWatcher(event) != nullptr);
}

TEST_F(TestTagController, can_open_file)
{
    ASSERT_NE(m_pController, nullptr);

    m_tagUrl.setTaggedFileUrl(tempDirPath_A);
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->openFile(event));
}

TEST_F(TestTagController, can_open_files)
{
    ASSERT_NE(m_pController, nullptr);

    DUrlList list;
    m_tagUrl.setTaggedFileUrl(tempDirPath_A);
    list.append(m_tagUrl);
    m_tagUrl.setTaggedFileUrl(tempDirPath_B);
    list.append(m_tagUrl);

    auto event = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, list);
    EXPECT_TRUE(m_pController->openFiles(event));
}

TEST_F(TestTagController, can_setFileTags)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, m_tagFileUrl, QList<QString>({"红色"}));
    EXPECT_TRUE(m_pController->setFileTags(event));
}

TEST_F(TestTagController, can_getTagsThroughFiles)
{
    auto event = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({m_tagFileUrl}));
    EXPECT_TRUE(!m_pController->getTagsThroughFiles(event).isEmpty());
}

TEST_F(TestTagController, can_removeTagsOfFile)
{
    auto event = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, m_tagFileUrl, QList<QString>({"红色"}));
    EXPECT_TRUE(m_pController->removeTagsOfFile(event));
}

TEST_F(TestTagController, can_openFileByApp)
{
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", m_tagFileUrl);
    EXPECT_TRUE(m_pController->openFileByApp(event));
}

TEST_F(TestTagController, can_openFilesByApp)
{
    auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << m_tagFileUrl);
    EXPECT_TRUE(m_pController->openFilesByApp(event));
}

TEST_F(TestTagController, can_openFileLocation)
{
    auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->openFileLocation(event));
}

//TEST_F(TestTagController, can_writeFilesToClipboard)
//{
//    auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << m_tagFileUrl);
//    EXPECT_TRUE(m_pController->writeFilesToClipboard(event));
//}

TEST_F(TestTagController, can_setPermissions)
{
    auto event = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, m_tagFileUrl, (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadOwner));
    EXPECT_TRUE(m_pController->setPermissions(event));
}

TEST_F(TestTagController, can_addToBookmark)
{
    auto event = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(!m_pController->addToBookmark(event));
}

TEST_F(TestTagController, can_removeBookmark)
{
    auto event = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(!m_pController->removeBookmark(event));
}

TEST_F(TestTagController, can_createSymlink)
{
    QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/tag_symlink.txt";
    auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, m_tagFileUrl, DUrl::fromLocalFile(symlinkPath));
    EXPECT_TRUE(m_pController->createSymlink(event));
    QProcess::execute("rm -f " + symlinkPath);
}

TEST_F(TestTagController, can_shareFolder)
{
    auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, m_tagUrl, "tag_test");
    EXPECT_TRUE(m_pController->shareFolder(event));
}

TEST_F(TestTagController, can_unShareFolder)
{
    auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, m_tagUrl, "tag_test");
    m_pController->shareFolder(event);

    auto unShareFolderEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->unShareFolder(unShareFolderEvent));
}

TEST_F(TestTagController, can_openInTerminal)
{
    auto event = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->openInTerminal(event));
}

TEST_F(TestTagController, can_renameFile)
{
    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + NEW_TAG_TXT_FILE;
    DUrl to;
    to.setScheme(SEARCH_SCHEME);
    to.setTaggedFileUrl(temp);
    auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, m_tagFileUrl, to);
    EXPECT_TRUE(m_pController->renameFile(event));
}

TEST_F(TestTagController, test_wind_up)
{
    ASSERT_NE(m_pManager, nullptr);
    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    m_pManager->deleteTags(tags);

    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + NEW_TAG_TXT_FILE;
    QProcess::execute("rm " + temp);
    QProcess::execute("rm -rf " + tempDirPath_A + " " + tempDirPath_B);
}

}
