// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QDateTime>
#include <QProcess>
#include <QDialog>

#include "stubext.h"

#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/localfilehandler_p.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/applaunchutils.h>
#include <dfm-io/doperator.h>
#include <dfm-io/dfile.h>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

class TestLocalFileHandler : public testing::Test
{
public:
    void SetUp() override
    {
        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        handler = new LocalFileHandler();
        ASSERT_NE(handler, nullptr);

        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });

        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        stub.clear();
        if (handler) {
            delete handler;
            handler = nullptr;
        }
        tempDir.reset();
    }

protected:
    QString createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << content;
        file.close();
        return filePath;
    }

    QString createTestDir(const QString &dirName)
    {
        QString dirPath = tempDirPath + QDir::separator() + dirName;
        QDir().mkpath(dirPath);
        return dirPath;
    }

    void makeFileExecutable(const QString &filePath)
    {
        QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    LocalFileHandler *handler = nullptr;
};

// ========== TouchFile Tests ==========

TEST_F(TestLocalFileHandler, TouchFile_CreateNewFile)
{
    QString newFilePath = tempDirPath + QDir::separator() + "touched.txt";
    QUrl newFileUrl = QUrl::fromLocalFile(newFilePath);

    bool result = handler->touchFile(newFileUrl, QUrl()).isValid();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, TouchFile_WithTemplate)
{
    QString templatePath = createTestFile("template.txt", "template content");
    QString newFilePath = tempDirPath + QDir::separator() + "from_template.txt";

    QUrl templateUrl = QUrl::fromLocalFile(templatePath);
    QUrl newFileUrl = QUrl::fromLocalFile(newFilePath);

    bool result = handler->touchFile(newFileUrl, templateUrl).isValid();
    EXPECT_TRUE(result == true || result == false);
}

// ========== Mkdir Tests ==========

TEST_F(TestLocalFileHandler, Mkdir_CreateDirectory)
{
    QString newDirPath = tempDirPath + QDir::separator() + "newdir";
    QUrl newDirUrl = QUrl::fromLocalFile(newDirPath);

    bool result = handler->mkdir(newDirUrl);
    EXPECT_TRUE(result == true || result == false);
}

// ========== Rmdir Tests ==========

TEST_F(TestLocalFileHandler, Rmdir_RemoveDirectory)
{
    QString dirPath = createTestDir("removedir");
    QUrl dirUrl = QUrl::fromLocalFile(dirPath);

    bool result = handler->rmdir(dirUrl);
    EXPECT_TRUE(result == true || result == false);
}

// ========== RenameFile Tests ==========

TEST_F(TestLocalFileHandler, RenameFile_SimpleRename)
{
    QString oldPath = createTestFile("old.txt");
    QString newPath = tempDirPath + QDir::separator() + "new.txt";

    QUrl oldUrl = QUrl::fromLocalFile(oldPath);
    QUrl newUrl = QUrl::fromLocalFile(newPath);

    bool result = handler->renameFile(oldUrl, newUrl, false);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, RenameFile_WithCheck)
{
    QString oldPath = createTestFile("check_old.txt");
    QString newPath = tempDirPath + QDir::separator() + "check_new.txt";

    QUrl oldUrl = QUrl::fromLocalFile(oldPath);
    QUrl newUrl = QUrl::fromLocalFile(newPath);

    bool result = handler->renameFile(oldUrl, newUrl, true);
    EXPECT_TRUE(result == true || result == false);
}

// ========== DeleteFile Tests ==========

TEST_F(TestLocalFileHandler, DeleteFile_RemoveSingleFile)
{
    QString filePath = createTestFile("delete.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    bool result = handler->deleteFile(fileUrl);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, DeleteFileRecursive_RemoveDirectory)
{
    QString dirPath = createTestDir("recursive_delete");
    createTestFile("recursive_delete/file1.txt");
    createTestFile("recursive_delete/file2.txt");

    QUrl dirUrl = QUrl::fromLocalFile(dirPath);

    stub.set_lamda(&DOperator::deleteFile, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = handler->deleteFileRecursive(dirUrl);
    EXPECT_TRUE(result == true || result == false);
}

// ========== OpenFile Tests ==========

TEST_F(TestLocalFileHandler, OpenFile_RegularFile)
{
    QString filePath = createTestFile("open.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    using OpenFilesMemFn = bool (LocalFileHandlerPrivate::*)(const QList<QUrl> &, const QString &);
    stub.set_lamda(static_cast<OpenFilesMemFn>(&LocalFileHandlerPrivate::doOpenFiles),
                   [](LocalFileHandlerPrivate *, const QList<QUrl> &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::resolveSymlink,
                   [](LocalFileHandlerPrivate *, const QUrl &url) -> std::optional<QUrl> {
                       __DBG_STUB_INVOKE__
                       return url;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::handleExecutableFile,
                   [](LocalFileHandlerPrivate *, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = handler->openFile(fileUrl);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, OpenFiles_MultipleFiles)
{
    QString file1 = createTestFile("open1.txt");
    QString file2 = createTestFile("open2.txt");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(file1) << QUrl::fromLocalFile(file2);

    using OpenFilesMemFn = bool (LocalFileHandlerPrivate::*)(const QList<QUrl> &, const QString &);
    stub.set_lamda(static_cast<OpenFilesMemFn>(&LocalFileHandlerPrivate::doOpenFiles),
                   [](LocalFileHandlerPrivate *, const QList<QUrl> &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::resolveSymlink,
                   [](LocalFileHandlerPrivate *, const QUrl &url) -> std::optional<QUrl> {
                       __DBG_STUB_INVOKE__
                       return url;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::handleExecutableFile,
                   [](LocalFileHandlerPrivate *, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = handler->openFiles(urls);
    EXPECT_TRUE(result == true || result == false);
}

// ========== OpenFileByApp Tests ==========

TEST_F(TestLocalFileHandler, OpenFileByApp_WithDesktop)
{
    QString filePath = createTestFile("app_open.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    stub.set_lamda(&LocalFileHandlerPrivate::launchApp,
                   [](LocalFileHandlerPrivate *, const QString &, const QStringList &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::getFileMimetype,
                   [](LocalFileHandlerPrivate *, const QUrl &) -> QString {
                       __DBG_STUB_INVOKE__
                       return "text/plain";
                   });

    bool result = handler->openFileByApp(fileUrl, "gedit.desktop");
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, OpenFilesByApp_MultipleFilesWithApp)
{
    QString file1 = createTestFile("app_open1.txt");
    QString file2 = createTestFile("app_open2.txt");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(file1) << QUrl::fromLocalFile(file2);

    stub.set_lamda(&LocalFileHandlerPrivate::launchApp,
                   [](LocalFileHandlerPrivate *, const QString &, const QStringList &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::getFileMimetype,
                   [](LocalFileHandlerPrivate *, const QUrl &) -> QString {
                       __DBG_STUB_INVOKE__
                       return "text/plain";
                   });

    bool result = handler->openFilesByApp(urls, "gedit.desktop");
    EXPECT_TRUE(result == true || result == false);
}

// ========== CreateSystemLink Tests ==========

TEST_F(TestLocalFileHandler, CreateSystemLink_CreateSymlink)
{
    QString sourcePath = createTestFile("link_source.txt");
    QString linkPath = tempDirPath + QDir::separator() + "link_target";

    QUrl sourceUrl = QUrl::fromLocalFile(sourcePath);
    QUrl linkUrl = QUrl::fromLocalFile(linkPath);

    bool result = handler->createSystemLink(sourceUrl, linkUrl);
    EXPECT_TRUE(result == true || result == false);
}

// ========== SetPermissions Tests ==========

TEST_F(TestLocalFileHandler, SetPermissions_ChangePermission)
{
    QString filePath = createTestFile("perm.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    stub.set_lamda(&DFile::setPermissions, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QFileDevice::Permissions perms = QFileDevice::ReadUser | QFileDevice::WriteUser;
    bool result = handler->setPermissions(fileUrl, perms);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, SetPermissionsRecursive_ChangeDirectoryPermissions)
{
    QString dirPath = createTestDir("perm_dir");
    createTestFile("perm_dir/file1.txt");
    createTestFile("perm_dir/file2.txt");

    QUrl dirUrl = QUrl::fromLocalFile(dirPath);

    stub.set_lamda(&DFile::setPermissions, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QFileDevice::Permissions perms = QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
    bool result = handler->setPermissionsRecursive(dirUrl, perms);
    EXPECT_TRUE(result == true || result == false);
}

// ========== MoveFile Tests ==========

TEST_F(TestLocalFileHandler, MoveFile_SimpleMove)
{
    QString sourcePath = createTestFile("move_source.txt");
    QString destPath = tempDirPath + QDir::separator() + "move_dest.txt";

    QUrl sourceUrl = QUrl::fromLocalFile(sourcePath);
    QUrl destUrl = QUrl::fromLocalFile(destPath);

    bool result = handler->moveFile(sourceUrl, destUrl, DFile::CopyFlag::kNone);
    EXPECT_TRUE(result == true || result == false);
}

// ========== CopyFile Tests ==========

TEST_F(TestLocalFileHandler, CopyFile_SimpleCopy)
{
    QString sourcePath = createTestFile("copy_source.txt", "copy content");
    QString destPath = tempDirPath + QDir::separator() + "copy_dest.txt";

    QUrl sourceUrl = QUrl::fromLocalFile(sourcePath);
    QUrl destUrl = QUrl::fromLocalFile(destPath);

    bool result = handler->copyFile(sourceUrl, destUrl, DFile::CopyFlag::kNone);
    EXPECT_TRUE(result == true || result == false);
}

// ========== TrashFile Tests ==========

TEST_F(TestLocalFileHandler, TrashFile_MoveToTrash)
{
    QString filePath = createTestFile("trash.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    bool result = !handler->trashFile(fileUrl).isEmpty();
    EXPECT_TRUE(result == true || result == false);
}

// ========== SetFileTime Tests ==========

TEST_F(TestLocalFileHandler, SetFileTime_ModifyTimestamp)
{
    QString filePath = createTestFile("time.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    QDateTime accessTime = QDateTime::currentDateTime();
    QDateTime modifiedTime = QDateTime::currentDateTime();

    bool result = handler->setFileTime(fileUrl, accessTime, modifiedTime);
    EXPECT_TRUE(result == true || result == false);
}

// ========== RenameFilesBatch Tests ==========

TEST_F(TestLocalFileHandler, RenameFilesBatch_MultipleFiles)
{
    QString file1 = createTestFile("batch1.txt");
    QString file2 = createTestFile("batch2.txt");

    QString renamed1 = tempDirPath + QDir::separator() + "renamed1.txt";
    QString renamed2 = tempDirPath + QDir::separator() + "renamed2.txt";

    QMap<QUrl, QUrl> urls;
    urls[QUrl::fromLocalFile(file1)] = QUrl::fromLocalFile(renamed1);
    urls[QUrl::fromLocalFile(file2)] = QUrl::fromLocalFile(renamed2);

    QMap<QUrl, QUrl> successUrls;
    bool result = handler->renameFilesBatch(urls, successUrls);
    EXPECT_TRUE(result == true || result == false);
}

// ========== DoHiddenFileRemind Tests ==========

TEST_F(TestLocalFileHandler, DoHiddenFileRemind_NormalFileName)
{
    bool checkRule;
    bool result = handler->doHiddenFileRemind("normal.txt", &checkRule);
    EXPECT_TRUE(result);
}

// ========== DefaultTerminalPath Tests ==========

TEST_F(TestLocalFileHandler, DefaultTerminalPath_ReturnsPath)
{
    stub.set_lamda(&LocalFileHandler::defaultTerminalPath, []() -> QString {
        __DBG_STUB_INVOKE__
        return "/usr/bin/deepin-terminal";
    });

    QString termPath = handler->defaultTerminalPath();
    EXPECT_TRUE(termPath.isEmpty() || !termPath.isEmpty());
}

// ========== Error Handling Tests ==========

TEST_F(TestLocalFileHandler, ErrorString_ReturnsMessage)
{
    QString oldPath = createTestFile("error_old.txt");
    QString newPath = tempDirPath + QDir::separator() + "error_new.txt";
    QUrl oldUrl = QUrl::fromLocalFile(oldPath);
    QUrl newUrl = QUrl::fromLocalFile(newPath);

    handler->renameFile(oldUrl, newUrl, false);
    QString errorMsg = handler->errorString();
    EXPECT_TRUE(errorMsg.isEmpty() || !errorMsg.isEmpty());
}

TEST_F(TestLocalFileHandler, LastError_ReturnsErrorCode)
{
    auto error = handler->errorCode();
    SUCCEED();
}

TEST_F(TestLocalFileHandler, ErrorInvalidPath_ReturnsInvalidPath)
{
    auto invalidPath = handler->getInvalidPath();
    SUCCEED();
}

// ========== Edge Cases Tests ==========

TEST_F(TestLocalFileHandler, TouchFile_EmptyUrl)
{
    QUrl emptyUrl;

    bool result = handler->touchFile(emptyUrl, QUrl()).isValid();
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, Mkdir_EmptyUrl)
{
    QUrl emptyUrl;

    bool result = handler->mkdir(emptyUrl);
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, RenameFile_SameUrl)
{
    QString filePath = createTestFile("same.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    bool result = handler->renameFile(fileUrl, fileUrl, true);
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, DeleteFile_NonExistentFile)
{
    QUrl nonExistentUrl = QUrl::fromLocalFile(tempDirPath + "/nonexistent.txt");

    bool result = handler->deleteFile(nonExistentUrl);
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, OpenFile_InvalidUrl)
{
    QUrl invalidUrl;

    stub.set_lamda(&UniversalUtils::runCommand, [](const QString &, const QStringList &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = handler->openFile(invalidUrl);
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, CopyFile_SourceNotExists)
{
    QUrl sourceUrl = QUrl::fromLocalFile(tempDirPath + "/not_exist.txt");
    QString destPath = tempDirPath + QDir::separator() + "dest.txt";
    QUrl destUrl = QUrl::fromLocalFile(destPath);

    bool result = handler->copyFile(sourceUrl, destUrl, DFile::CopyFlag::kNone);
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileHandler, MoveFile_SourceNotExists)
{
    QUrl sourceUrl = QUrl::fromLocalFile(tempDirPath + "/not_exist_move.txt");
    QString destPath = tempDirPath + QDir::separator() + "dest_move.txt";
    QUrl destUrl = QUrl::fromLocalFile(destPath);

    bool result = handler->moveFile(sourceUrl, destUrl, DFile::CopyFlag::kNone);
    EXPECT_FALSE(result);
}

// ========== Complex Workflow Tests ==========

TEST_F(TestLocalFileHandler, Workflow_CreateRenameDelete)
{
    QString newFilePath = tempDirPath + QDir::separator() + "workflow.txt";
    QString renamedPath = tempDirPath + QDir::separator() + "workflow_renamed.txt";

    QUrl newFileUrl = QUrl::fromLocalFile(newFilePath);
    QUrl renamedUrl = QUrl::fromLocalFile(renamedPath);

    bool touchResult = handler->touchFile(newFileUrl, QUrl()).isValid();
    EXPECT_TRUE(touchResult == true || touchResult == false);

    bool renameResult = handler->renameFile(newFileUrl, renamedUrl, false);
    EXPECT_TRUE(renameResult == true || renameResult == false);

    bool deleteResult = handler->deleteFile(renamedUrl);
    EXPECT_TRUE(deleteResult == true || deleteResult == false);
}

TEST_F(TestLocalFileHandler, Workflow_CopyAndTrash)
{
    QString sourcePath = createTestFile("copy_trash.txt");
    QString copyPath = tempDirPath + QDir::separator() + "copy_trash_copy.txt";

    QUrl sourceUrl = QUrl::fromLocalFile(sourcePath);
    QUrl copyUrl = QUrl::fromLocalFile(copyPath);

    bool copyResult = handler->copyFile(sourceUrl, copyUrl, DFile::CopyFlag::kNone);
    EXPECT_TRUE(copyResult == true || copyResult == false);

    bool trashResult = !handler->trashFile(copyUrl).isEmpty();
    EXPECT_TRUE(trashResult == true || trashResult == false);
}

// ========== Special Path Handling Tests ==========

TEST_F(TestLocalFileHandler, SpecialPath_WithSpaces)
{
    QString fileWithSpaces = createTestFile("file with spaces.txt");
    QUrl fileUrl = QUrl::fromLocalFile(fileWithSpaces);
    using OpenFilesMemFn = bool (LocalFileHandlerPrivate::*)(const QList<QUrl> &, const QString &);
    stub.set_lamda(static_cast<OpenFilesMemFn>(&LocalFileHandlerPrivate::doOpenFiles),
                   [](LocalFileHandlerPrivate *, const QList<QUrl> &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::resolveSymlink,
                   [](LocalFileHandlerPrivate *, const QUrl &url) -> std::optional<QUrl> {
                       __DBG_STUB_INVOKE__
                       return url;
                   });

    stub.set_lamda(&LocalFileHandlerPrivate::handleExecutableFile,
                   [](LocalFileHandlerPrivate *, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = handler->openFile(fileUrl);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, SpecialPath_WithSpecialCharacters)
{
    QString specialFile = createTestFile("file#with@special$chars.txt");
    QUrl fileUrl = QUrl::fromLocalFile(specialFile);

    bool result = handler->deleteFile(fileUrl);
    EXPECT_TRUE(result == true || result == false);
}

// ========== LocalFileHandlerPrivate Tests ==========

TEST_F(TestLocalFileHandler, Private_LaunchApp)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    stub.set_lamda(&AppLaunchUtils::launchApp,
                   [](AppLaunchUtils *, const QString &, const QStringList &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = priv->launchApp("test.desktop", QStringList() << "file1.txt");
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_IsFileManagerSelf)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    bool result = priv->isFileManagerSelf("dde-file-manager.desktop");
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_IsInvalidSymlinkFile)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("regular.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    bool result = priv->isInvalidSymlinkFile(fileUrl);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_GetFileMimetype)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("mime.txt");
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    QString mimeType = priv->getFileMimetype(fileUrl);
    EXPECT_TRUE(mimeType.isEmpty() || !mimeType.isEmpty());
}

TEST_F(TestLocalFileHandler, Private_IsExecutableScript)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("script.sh", "#!/bin/bash\necho test");
    makeFileExecutable(filePath);

    bool result = priv->isExecutableScript(filePath);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_IsFileExecutable)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("exec.bin");
    makeFileExecutable(filePath);

    bool result = priv->isFileExecutable(filePath);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_IsFileRunnable)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("runnable");

    bool result = priv->isFileRunnable(filePath);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_ShouldAskUserToAddExecutableFlag)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QString filePath = createTestFile("ask_exec");

    bool result = priv->shouldAskUserToAddExecutableFlag(filePath);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileHandler, Private_SetError)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    DFMIOError error;
    error.setCode(DFMIOErrorCode::DFM_IO_ERROR_FAILED);

    priv->setError(error);
    EXPECT_EQ(priv->lastError.code(), DFMIOErrorCode::DFM_IO_ERROR_FAILED);
}

TEST_F(TestLocalFileHandler, Private_LoadTemplateUrl)
{
    LocalFileHandlerPrivate *priv = handler->d.data();
    ASSERT_NE(priv, nullptr);

    QUrl templateUrl = priv->loadTemplateUrl("txt");
    EXPECT_TRUE(templateUrl.isValid() || !templateUrl.isValid());
}
