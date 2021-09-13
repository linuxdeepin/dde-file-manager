#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "controllers/filecontroller.cpp"
#include "stub.h"
#include "stubext.h"
#include "testhelper.h"
#include "dfmevent.h"
#include "gvfs/gvfsmountmanager.h"

namespace  {
static Stub *stub;

class FileControllerTest : public testing::Test
{
public:

    static void SetUpTestCase()
    {
        typedef bool (*OpenFileWithFlag)(const QString & path, int flag);
        typedef bool (*OpenFile)(const QString & path);
        typedef bool (*OpenFiles)(const QStringList & filePaths);
        typedef bool (*OpenFilesWithArgs)(const QString & file, const QStringList & filePaths);
        typedef QString(*GetPath)();

        // 打桩，避免弹框
        OpenFileWithFlag openFileWithFlag = [](const QString & path, int flag) {
            Q_UNUSED(path);
            Q_UNUSED(flag);
            return true;
        };
        OpenFile openFile = [](const QString & path) {
            Q_UNUSED(path);
            return true;
        };
        OpenFiles openFiles = [](const QStringList & filePaths) {
            Q_UNUSED(filePaths);
            return true;
        };
        OpenFilesWithArgs openFilesWithArgs = [](const QString & file, const QStringList & filePaths) {
            Q_UNUSED(file);
            Q_UNUSED(filePaths);
            return true;
        };
        GetPath getPath = []() {
            return QString("/bin/touch");
        };

        stub = new Stub;
        stub->set(static_cast<OpenFileWithFlag>(&FileUtils::openExcutableScriptFile), openFileWithFlag);
        stub->set(static_cast<OpenFileWithFlag>(&FileUtils::openExcutableFile), openFileWithFlag);
        stub->set(static_cast<OpenFileWithFlag>(&FileUtils::addExecutableFlagAndExecuse), openFileWithFlag);
        stub->set(static_cast<OpenFile>(&FileUtils::openFile), openFile);
        stub->set(static_cast<OpenFiles>(&FileUtils::openFiles), openFiles);
        stub->set(static_cast<OpenFilesWithArgs>(&FileUtils::openFilesByApp), openFilesWithArgs);
        stub->set(static_cast<GetPath>(&FileUtils::defaultTerminalPath), getPath);
    }

    static void TearDownTestCase()
    {
        if (stub) {
            delete stub;
            stub = nullptr;
        }
    }

    virtual void SetUp() override
    {
        controller = new FileController();

        fileName = TestHelper::createTmpFile("normal_file");
        linkName = TestHelper::createTmpSymlinkFile(fileName);

        scriptName = TestHelper::createTmpFile("script_file.sh");
        chmod(scriptName.toStdString().c_str(), 0700);

        pasteDir = TestHelper::createTmpDir();
        shareDir = TestHelper::createTmpDir();
        cpDstFileName = TestHelper::createTmpDir();
    }

    virtual void TearDown() override
    {
        qApp->processEvents();

        delete controller;
        controller = nullptr;

        TestHelper::deleteTmpFiles({linkName,
                                    fileName,
                                    scriptName,
                                    pasteDir,
                                    shareDir,
                                    cpDstFileName
                                   });
    }

    void DumpDirector(DDirIteratorPointer director)
    {
        auto fileName = director->fileName();
        auto fileUrl = director->fileUrl();
        auto fileInfo = director->fileInfo();
        auto durl = director->url();

        QString keyword("keyword");
        director->enableIteratorByKeyword(keyword);

        while (director->hasNext()) {
            director->next();
        }
    }

    FileController *controller;
    QString fileName;
    QString linkName;
    QString exeName = "/bin/touch";
    QString scriptName;
    QString pasteDir;
    QString shareDir;
    QString cpDstFileName;
    QString noPermissionDir = "/Permisson/";
    QString noPermissionFile = "/Permisson";
    QString desktopFile = "/usr/share/applications/dde-computer.desktop";
};
}

TEST_F(FileControllerTest, tst_find_executable)
{
    bool found = controller->findExecutable("bash", {"/usr/bin/"});
    EXPECT_TRUE(found);
}

TEST_F(FileControllerTest, tst_create_file_info)
{
    // 普通文件
    DUrl url = DUrl::fromLocalFile("/tmp/");
    DAbstractFileInfoPointer fp1 = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_TRUE(fp1 != nullptr);

    // 桌面文件
    url = DUrl::fromLocalFile(desktopFile);
    DAbstractFileInfoPointer fp2 = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_TRUE(fp2 != nullptr);

    // 挂载文件
    url = DUrl("smb://127.0.0.1/");
    url.setScheme(SMB_SCHEME);
    DFMUrlBaseEvent event(nullptr, url);
    // GvfsMountManager::mount_sync(event);
    DAbstractFileInfoPointer fp3 = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_TRUE(fp3 != nullptr);
}

TEST_F(FileControllerTest, tst_create_director)
{
    // sorted
    DUrl url = DUrl::fromLocalFile("/tmp/");
    bool gvfsFile = false;
    QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(), QDir::AllEntries, flags, false, gvfsFile);
    DDirIteratorPointer dirIterator = controller->createDirIterator(event);
    EXPECT_TRUE(dirIterator != nullptr);
    DumpDirector(dirIterator);

    // no sort
    flags = QDirIterator::NoIteratorFlags;
    event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(), QDir::AllEntries, flags, false, gvfsFile);
    dirIterator = controller->createDirIterator(event);
    EXPECT_TRUE(dirIterator != nullptr);
    DumpDirector(dirIterator);
}

TEST_F(FileControllerTest, tst_open_file)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrl url = DUrl::fromLocalFile(linkName);
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, url);
    bool open = controller->openFile(event);
    EXPECT_TRUE(open);

    url = DUrl::fromLocalFile(fileName);
    event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, url);
    open = controller->openFile(event);
    EXPECT_TRUE(open);

    url = DUrl::fromLocalFile(scriptName);
    event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, url);
    open = controller->openFile(event);
    EXPECT_TRUE(open);

    url = DUrl::fromLocalFile(exeName);
    event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, url);
    open = controller->openFile(event);
    EXPECT_TRUE(open);
}

TEST_F(FileControllerTest, tst_open_files)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrlList list;
    list.append(DUrl::fromLocalFile(fileName));
    list.append(DUrl::fromLocalFile(linkName));
    list.append(DUrl::fromLocalFile(scriptName));
    list.append(DUrl::fromLocalFile(exeName));
    auto event = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, list);
    bool open = controller->openFiles(event);
    EXPECT_TRUE(open);
}

TEST_F(FileControllerTest, tst_open_file_by_app)
{
    DUrl url = DUrl::fromLocalFile(linkName);
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, exeName, url);
    bool open = controller->openFileByApp(event);
    EXPECT_TRUE(open);
}

TEST_F(FileControllerTest, tst_open_files_by_app)
{
    DUrlList list;
    list.append(DUrl::fromLocalFile(fileName));
    list.append(DUrl::fromLocalFile(linkName));
    auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, exeName, list);
    bool open = controller->openFilesByApp(event);
    EXPECT_TRUE(open);
}

TEST_F(FileControllerTest, tst_open_compress_files)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrlList list;
    list.append(DUrl::fromLocalFile(fileName));
    list.append(DUrl::fromLocalFile(linkName));
    auto event = dMakeEventPointer<DFMCompressEvent>(nullptr, list);
    EXPECT_NO_FATAL_FAILURE(controller->compressFiles(event));
}

TEST_F(FileControllerTest, tst_decompress_files)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrlList list;
    list.append(DUrl::fromLocalFile(fileName));
    list.append(DUrl::fromLocalFile(linkName));
    auto event = dMakeEventPointer<DFMDecompressEvent>(nullptr, list);
    EXPECT_NO_FATAL_FAILURE(controller->decompressFile(event));
}

TEST_F(FileControllerTest, tst_decompress_file_here)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrlList list;
    list.append(DUrl::fromLocalFile(fileName));
    list.append(DUrl::fromLocalFile(linkName));
    auto event = dMakeEventPointer<DFMDecompressEvent>(nullptr, list);
    EXPECT_NO_FATAL_FAILURE(controller->decompressFileHere(event));
}

TEST_F(FileControllerTest, tst_write_files_to_clipboard)
{
    DUrlList list;
    list << DUrl::fromLocalFile(fileName);
    bool result = controller->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, list));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_rename_file)
{
    DUrl from = DUrl::fromLocalFile(fileName).toAbsolutePathUrl();
    DUrl to = DUrl::fromLocalFile(cpDstFileName).toAbsolutePathUrl();
    bool result = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, from, to));
    EXPECT_TRUE(result);

    from = DUrl::fromLocalFile(desktopFile).toAbsolutePathUrl();
    to = DUrl::fromLocalFile(cpDstFileName).toAbsolutePathUrl();
    result = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, from, to));
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, tst_delete_files)
{
    // 拷贝过程, 阻塞CI
    // DUrlList list;
    // list.append(DUrl::fromLocalFile(fileName));
    // list.append(DUrl::fromLocalFile(scriptName));
    // bool result = controller->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, list));
    // EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_move_to_trash)
{
    // 拷贝过程, 阻塞CI
    // DUrlList list;
    // list.append(DUrl::fromLocalFile(fileName));
    // list.append(DUrl::fromLocalFile(scriptName));
    // auto rList = controller->moveToTrash(dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, list));
    // EXPECT_TRUE(!rList.empty());
}

TEST_F(FileControllerTest, tst_paste_file)
{
    // 拷贝过程, 阻塞CI
    // DUrlList list;
    // list.append(DUrl::fromLocalFile(fileName));
    // DUrl dir = DUrl::fromLocalFile(pasteDir);
    // auto rList = controller->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::ClipboardAction::CopyAction, dir, list));
    // EXPECT_TRUE(rList.empty());
}

TEST_F(FileControllerTest, tst_paste_file_v1)
{
    // 拷贝过程, 阻塞CI
    // DUrlList list;
    // list.append(DUrl::fromLocalFile(fileName));
    // DUrl dir = DUrl::fromLocalFile(pasteDir);
    // auto rList = pasteFilesV1(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::ClipboardAction::CopyAction, dir, list));
    // EXPECT_TRUE(!rList.empty());
}

TEST_F(FileControllerTest, tst_mkdir)
{
    // 阻塞CI
    // DUrl dir = DUrl::fromLocalFile(pasteDir);
    // bool result = controller->mkdir(dMakeEventPointer<DFMMkdirEvent>(nullptr, dir));
    // EXPECT_TRUE(result);

    // dir = DUrl::fromLocalFile(noPermissionDir);
    // result = controller->mkdir(dMakeEventPointer<DFMMkdirEvent>(nullptr, dir));
    // EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, tst_touch)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrl url = DUrl::fromLocalFile(fileName);
    bool result = controller->touch(dMakeEventPointer<DFMTouchFileEvent>(nullptr, url));
    EXPECT_TRUE(result);

    url = DUrl::fromLocalFile(noPermissionFile);
    result = controller->touch(dMakeEventPointer<DFMTouchFileEvent>(nullptr, url));
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, tst_set_permissions)
{
    DUrl url = DUrl::fromLocalFile(fileName).toAbsolutePathUrl();
    bool result = controller->setPermissions(dMakeEventPointer<DFMSetPermissionEvent>(nullptr, url, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_share_folder)
{
    // 阻塞CI
    // DUrl url = DUrl::fromLocalFile(shareDir).toAbsolutePathUrl();
    // bool result = controller->shareFolder(dMakeEventPointer<DFMFileShareEvent>(nullptr, url, "share_name"));
    // EXPECT_TRUE(result);

    // result = controller->unShareFolder(dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, url));
    // EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_open_in_terminal)
{
    bool (*st_startDetached)(void *, const QString &) = [](void *, const QString &) {
        return true;
    };

    typedef bool(*FunPtr)(const QString &);
    Stub stub;
    stub.set(FunPtr(&QProcess::startDetached), st_startDetached);

    DUrl url = DUrl::fromLocalFile(fileName);
    bool result = controller->openInTerminal(dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, url));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_book_mark)
{
    stub_ext::StubExt stubext;
    stubext.set_lamda(&DFileService::touchFile, []() { return true; });
    stubext.set_lamda(&DFileService::deleteFiles, []() { return true; });

    DUrl url = DUrl::fromLocalFile(fileName);
    bool result = controller->addToBookmark(dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, url));
    EXPECT_TRUE(result);

    result = controller->removeBookmark(dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, url));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_create_symlink)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [] { return (int)QDialog::Rejected; });

    DUrl fromUrl = DUrl::fromLocalFile(fileName).toAbsolutePathUrl();
    DUrl toUrl = DUrl::fromLocalFile(linkName).toAbsolutePathUrl();
    bool result = controller->createSymlink(dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fromUrl, toUrl));
    EXPECT_TRUE(!result);

    QDir dir;
    dir.remove(linkName);
    toUrl = DUrl::fromLocalFile(linkName).toAbsolutePathUrl();
    result = controller->createSymlink(dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fromUrl, toUrl));
    EXPECT_TRUE(result);
}

TEST_F(FileControllerTest, tst_create_file_watcher)
{
    DUrl url = DUrl::fromLocalFile("/tmp/");
    DAbstractFileWatcher *watcher = controller->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url));
    EXPECT_TRUE(watcher != nullptr);
    if (watcher)
        delete watcher;
}

TEST_F(FileControllerTest, tst_create_file_device)
{
    DUrl url = DUrl::fromLocalFile("/dev/urandom");
    auto *device = controller->createFileDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(device != nullptr);
    if (device)
        delete device;
}

TEST_F(FileControllerTest, tst_create_file_handler)
{
    DUrl url = DUrl::fromLocalFile(fileName);
    auto *handler = controller->createFileHandler(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(handler != nullptr);
    if (handler)
        delete handler;
}

TEST_F(FileControllerTest, tst_create_storage_info)
{
    DUrl url = DUrl::fromLocalFile("/dev/urandom");
    DStorageInfo *info = controller->createStorageInfo(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
    EXPECT_TRUE(info != nullptr);
    if (info)
        delete info;
}

TEST_F(FileControllerTest, tst_custom_hideen_file_match)
{
    bool result = FileController::customHiddenFileMatch("/tmp/", "test");
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, tst_private_file_match)
{
    bool result = FileController::privateFileMatch("/tmp/", "test");
    EXPECT_TRUE(!result);
}

TEST_F(FileControllerTest, tst_tags_manage)
{
    DUrl url = DUrl::fromLocalFile(fileName);
    QList<QString> tags;
    tags.append("important");
    auto result = controller->setFileTags(dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, url, tags));
    EXPECT_NO_FATAL_FAILURE(result);

    QList<DUrl> list;
    list.append(url);
    auto result1 = controller->getTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, list));
    EXPECT_NO_FATAL_FAILURE(!result1.empty());

    auto result2 = controller->removeTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, url, tags));
    EXPECT_NO_FATAL_FAILURE(result2);
}

TEST_F(FileControllerTest, tst_check_duplicate_name)
{
    auto result = controller->checkDuplicateName(fileName);
    EXPECT_TRUE(!result.isEmpty());
}

TEST_F(FileControllerTest, tst_file_added)
{
    DUrl url = DUrl::fromMTPFile("/mtp:");
    auto result = controller->fileAdded(url);
    EXPECT_TRUE(result);
}
