#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/filecontroller.h"
#include "dfmevent.h"

namespace  {
    class FileControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            controller = new FileController();

            urls.append(DUrl::fromLocalFile("/usr/"));
            urls.append(DUrl::fromSMBFile("smb://"));
            urls.append(DUrl::fromTrashFile("trash://"));
        }

        virtual void TearDown() override
        {
            delete controller;
            controller = nullptr;
            urls.clear();
        }

        void DumpDirector(DDirIteratorPointer director)
        {
            auto fileName = director->fileName();
            auto fileUrl = director->fileUrl();
            auto fileInfo = director->fileInfo();
            auto durl = director->url();

            while (director->hasNext()) {
                director->next();
            }
        }

        FileController *controller;
        QList<DUrl> urls;
    };
}

TEST_F(FileControllerTest, find_executable)
{
    bool find = controller->findExecutable("bash", {"/usr/bin/"});
    EXPECT_TRUE(find == true);
}

TEST_F(FileControllerTest, create_file_info)
{
    foreach (const DUrl& url, urls) {
        auto pfile = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
        EXPECT_TRUE(pfile != nullptr);
    }
}

TEST_F(FileControllerTest, create_director)
{
    bool isgvfs = false;
    QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    foreach (const DUrl& url, urls) {
        auto director = controller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(), QDir::AllEntries, flags, false, isgvfs));
        EXPECT_TRUE(director != nullptr);
        DumpDirector(director);
    }
}

TEST_F(FileControllerTest, create_director_no_sort)
{
    bool isgvfs = false;
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    foreach (const DUrl& url, urls) {
        auto director = controller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(), QDir::AllEntries, flags, false, isgvfs));
        EXPECT_TRUE(director != nullptr);
        DumpDirector(director);
    }
}

TEST_F(FileControllerTest, create_director_gvfs)
{
    bool isgvfs = true;
    QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    foreach (const DUrl& url, urls) {
        auto director = controller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(), QDir::AllEntries, flags, false, isgvfs));
        EXPECT_TRUE(director != nullptr);
        DumpDirector(director);
    }
}

TEST_F(FileControllerTest, write_files_to_clipboard)
{
    DUrlList list;
    list << DUrl::fromLocalFile("/etc/apt/sources.list");
    bool result = controller->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, list));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, rename_file)
{
    system("cp /etc/apt/sources.list /tmp/sources.list");
    DUrl from = DUrl::fromLocalFile("/tmp/sources.list").toAbsolutePathUrl();
    DUrl to = DUrl::fromLocalFile("/tmp/sources.list.bak").toAbsolutePathUrl();
    bool result = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, from, to));
    system("rm /tmp/sources.list.bak /tmp/sources.list");
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, delete_files)
{
    system("cp /etc/apt/sources.list /tmp/sources.list");
    system("cp /tmp/sources.list /tmp/sources_1.list");
    system("cp /tmp/sources.list /tmp/sources_2.list");

    DUrlList list;
    list.append(DUrl::fromLocalFile("/tmp/sources_1.list"));
    list.append(DUrl::fromLocalFile("/tmp/sources_2.list"));
    list.append(DUrl::fromLocalFile("/tmp/sources.list"));
    bool result = controller->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, list));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, move_to_trash)
{
    system("cp /etc/apt/sources.list /tmp/sources.list");
    system("cp /tmp/sources.list /tmp/sources_1.list");
    system("cp /tmp/sources.list /tmp/sources_2.list");
    DUrlList list;
    list.append(DUrl::fromLocalFile("/tmp/sources_1.list"));
    list.append(DUrl::fromLocalFile("/tmp/sources_2.list"));
    list.append(DUrl::fromLocalFile("/tmp/sources.list"));
    auto rList = controller->moveToTrash(dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, list));
    EXPECT_TRUE(!rList.empty());
}

TEST_F(FileControllerTest, paste_file)
{
//    system("mkdir /tmp/PasteDir");
//    DUrlList list;
//    list.append(DUrl::fromLocalFile("/etc/apt/sources.list"));
//    DUrl dir;
//    dir = DUrl::fromLocalFile("PasteDir");
//    auto rList = controller->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::ClipboardAction::CopyAction, dir, list));
//    EXPECT_TRUE(rList.empty());
}

TEST_F(FileControllerTest, mkdir)
{
    DUrl dir = DUrl::fromLocalFile("/tmp/MkDir");
    system("rm -rf /tmp/MkDir");
    bool result = controller->mkdir(dMakeEventPointer<DFMMkdirEvent>(nullptr, dir));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, touch)
{
    DUrl url = DUrl::fromLocalFile("/tmp/touch_file");
    bool result = controller->touch(dMakeEventPointer<DFMTouchFileEvent>(nullptr, url));
    system("rm /tmp/touch_file");
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, set_permissions)
{
    DUrl url = DUrl::fromLocalFile("/tmp/permission_file").toAbsolutePathUrl();
    system("touch /tmp/permission_file");
    bool result = controller->setPermissions(dMakeEventPointer<DFMSetPermissionEvent>(nullptr, url, QFileDevice::ReadOwner));
    system("rm -rf /tmp/permission_file");
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, share_folder)
{
    DUrl url = DUrl::fromLocalFile("/tmp/share_folder").toAbsolutePathUrl();
    system("mkdir /tmp/share_folder");
    bool result = controller->shareFolder(dMakeEventPointer<DFMFileShareEvent>(nullptr, url, "test_my_share_name"));
    EXPECT_TRUE(result);
    result = controller->unShareFolder(dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, url));
    system("rm -rf /tmp/share_folder");
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, open_in_terminal)
{
    DUrl url = DUrl::fromLocalFile("/ect/apt/");
    bool result = controller->openInTerminal(dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, url));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, add_to_book_mark)
{
    DUrl url = DUrl::fromLocalFile("/etc/apt/");
    bool result = controller->addToBookmark(dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, url));
    EXPECT_TRUE(!result);
    result = controller->removeBookmark(dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, url));
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, create_symlink)
{
    DUrl fromUrl = DUrl::fromLocalFile("/tmp/link_from_file").toAbsolutePathUrl();
    system("touch /tmp/link_from_file");
    DUrl toUrl = DUrl::fromLocalFile("/tmp/link_to_file").toAbsolutePathUrl();
    bool result = controller->createSymlink(dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fromUrl, toUrl));
    system("rm /tmp/link_to_file /tmp/link_from_file");
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, create_file_watcher)
{
    DUrl url = DUrl::fromLocalFile("/etc/apt/");
    DAbstractFileWatcher *watcher = controller->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url));
    EXPECT_TRUE(watcher != nullptr);
}

TEST_F(FileControllerTest, create_file_device)
{
    DUrl url = DUrl::fromLocalFile("/dev/urandom");
    auto *device = controller->createFileDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(device != nullptr);
}

TEST_F(FileControllerTest, create_file_handler)
{
    DUrl url = DUrl::fromLocalFile("/dev/urandom");
    auto *handler = controller->createFileHandler(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(handler != nullptr);
}

TEST_F(FileControllerTest, create_storage_info)
{
    DUrl url = DUrl::fromLocalFile("/dev/urandom");
    auto *info = controller->createStorageInfo(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(info != nullptr);
}

TEST_F(FileControllerTest, custom_hideen_file_match)
{
    bool result = FileController::customHiddenFileMatch("/tmp/", "test");
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, private_file_match)
{
    bool result = FileController::privateFileMatch("/tmp/", "test");
    EXPECT_TRUE(!result);
}

// setFileTags getTagsThroughFiles removeTagsOfFile
TEST_F(FileControllerTest, tags_manage)
{
    DUrl url = DUrl::fromLocalFile("/etc/apt/source.list");
    QList<QString> tags;
    tags.append("important");
    auto result = controller->setFileTags(dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, url, tags));
    EXPECT_TRUE(result);

    QList<DUrl> list;
    list.append(url);
    auto result1 = controller->getTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, list));
    EXPECT_TRUE(!result1.empty());

    auto result2 = controller->removeTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, url, tags));
    EXPECT_TRUE(result2);
}
