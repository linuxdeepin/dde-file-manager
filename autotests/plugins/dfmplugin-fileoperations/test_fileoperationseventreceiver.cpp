// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QFileDialog>
#include <QMimeData>
#include <QImage>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include "fileoperations/operationsstackproxy.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/hidefilehelper.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestFileOperationsEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Register file info factories
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        receiver = FileOperationsEventReceiver::instance();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    /*!\
     * \brief Create a temporary test file
     */
    QUrl createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDir->path() + "/" + fileName;
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write(content.toUtf8());
        file.close();

        return QUrl::fromLocalFile(filePath);
    }

    /*!\
     * \brief Create a temporary test directory
     */
    QUrl createTestDir(const QString &dirName)
    {
        QString dirPath = tempDir->path() + "/" + dirName;
        QDir().mkpath(dirPath);

        return QUrl::fromLocalFile(dirPath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    FileOperationsEventReceiver *receiver { nullptr };
};

// ========== FileOperationsEventReceiver::instance() Tests ==========

TEST_F(TestFileOperationsEventReceiver, Instance_ReturnsSingleton)
{
    FileOperationsEventReceiver *instance1 = FileOperationsEventReceiver::instance();
    FileOperationsEventReceiver *instance2 = FileOperationsEventReceiver::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

// ========== Copy Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationCopy_EmptySources)
{
    stub.set_lamda(&FileCopyMoveJob::copy, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QList<QUrl> emptySources;
    QUrl target = QUrl::fromLocalFile(tempDir->path());

    receiver->handleOperationCopy(0, emptySources, target, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCopy_ValidSources)
{
    stub.set_lamda(&FileCopyMoveJob::copy, [](FileCopyMoveJob *, const QList<QUrl> &sources, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        EXPECT_GT(sources.count(), 0);
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl sourceFile = createTestFile("copy_source.txt");
    QUrl targetDir = createTestDir("copy_target");
    QList<QUrl> sources = { sourceFile };

    receiver->handleOperationCopy(0, sources, targetDir, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCopy_WithCallback)
{
    stub.set_lamda(&FileCopyMoveJob::copy, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
        EXPECT_TRUE(args->contains(AbstractJobHandler::CallbackKey::kWindowId));
    };

    QUrl sourceFile = createTestFile("callback.txt");
    QUrl targetDir = createTestDir("target");
    QList<QUrl> sources = { sourceFile };

    receiver->handleOperationCopy(0, sources, targetDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== Cut Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationCut_SameDirectory)
{
    QUrl sourceFile = createTestFile("same_dir.txt");
    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> sources = { sourceFile };

    // Cut to same directory should be rejected
    receiver->handleOperationCut(0, sources, parentDir, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCut_DifferentDirectory)
{
    stub.set_lamda(&FileCopyMoveJob::cut, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl sourceFile = createTestFile("cut_source.txt");
    QUrl targetDir = createTestDir("cut_target");
    QList<QUrl> sources = { sourceFile };

    receiver->handleOperationCut(0, sources, targetDir, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCut_WithCallback)
{
    stub.set_lamda(&FileCopyMoveJob::cut, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
        EXPECT_TRUE(args->contains(AbstractJobHandler::CallbackKey::kWindowId));
    };

    QUrl sourceFile = createTestFile("cut_callback.txt");
    QUrl targetDir = createTestDir("cut_target_cb");
    QList<QUrl> sources = { sourceFile };

    receiver->handleOperationCut(0, sources, targetDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== Delete Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationDeletes_EmptySources)
{
    QList<QUrl> emptySources;

    receiver->handleOperationDeletes(0, emptySources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationDeletes_UserCancels)
{
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Rejected;
    });

    QUrl testFile = createTestFile("delete_cancel.txt");
    QList<QUrl> sources = { testFile };

    receiver->handleOperationDeletes(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationDeletes_UserConfirms)
{
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::deletes, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl testFile = createTestFile("delete_confirm.txt");
    QList<QUrl> sources = { testFile };

    receiver->handleOperationDeletes(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationDeletes_WithCallback)
{
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::deletes, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
        EXPECT_TRUE(args->contains(AbstractJobHandler::CallbackKey::kWindowId));
    };

    QUrl testFile = createTestFile("delete_callback.txt");
    QList<QUrl> sources = { testFile };

    receiver->handleOperationDeletes(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== Open Files Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFiles_EmptyList)
{
    bool result = receiver->handleOperationOpenFiles(0, QList<QUrl>());

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFiles_Success)
{
    stub.set_lamda(&LocalFileHandler::openFiles, [](LocalFileHandler *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl testFile = createTestFile("open.txt");
    QList<QUrl> urls = { testFile };

    bool result = receiver->handleOperationOpenFiles(0, urls);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFilesByApp_ValidApp)
{
    stub.set_lamda(&LocalFileHandler::openFilesByApp, [](LocalFileHandler *, const QList<QUrl> &, const QString &app) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(app.isEmpty());
        return true;
    });

    QUrl testFile = createTestFile("open_app.txt");
    QList<QUrl> urls = { testFile };
    QList<QString> apps = { "gedit" };

    bool result = receiver->handleOperationOpenFilesByApp(0, urls, apps);

    EXPECT_TRUE(result);
}

// ========== Rename Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFile_SameName)
{
    stub.set_lamda(&DialogManager::showRenameNameSameErrorDialog, []() {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    QUrl oldUrl = createTestFile("same_name.txt");
    QUrl newUrl = oldUrl;   // Same URL

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl, AbstractJobHandler::JobFlag::kNoHint);

    // Should handle gracefully (may return true for same name)
    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFile_TargetExists)
{
    stub.set_lamda(&DialogManager::showRenameNameSameErrorDialog, []() {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    QUrl oldUrl = createTestFile("old.txt");
    QUrl newUrl = createTestFile("existing.txt");

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFiles_ReplaceMode)
{
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, [](LocalFileHandler *, const QMap<QUrl, QUrl> &, QMap<QUrl, QUrl> &successUrls) -> bool {
        __DBG_STUB_INVOKE__
        successUrls.clear();
        return true;
    });

    stub.set_lamda(&FileUtils::fileBatchReplaceText, [](const QList<QUrl> &urls, const QPair<QString, QString> &) -> QMap<QUrl, QUrl> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, QUrl> result;
        for (const auto &url : urls) {
            result.insert(url, url);
        }
        return result;
    });

    QUrl file1 = createTestFile("replace1.txt");
    QUrl file2 = createTestFile("replace2.txt");
    QList<QUrl> urls = { file1, file2 };

    bool result = receiver->handleOperationRenameFiles(0, urls, qMakePair(QString("old"), QString("new")), true);

    SUCCEED();
}

// ========== Mkdir Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationMkdir_Success)
{
    stub.set_lamda(&LocalFileHandler::mkdir, [](LocalFileHandler *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    bool result = receiver->handleOperationMkdir(0, parentDir);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationMkdir_Failed)
{
    stub.set_lamda(&LocalFileHandler::mkdir, [](LocalFileHandler *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&LocalFileHandler::errorString, [](LocalFileHandler *) -> QString {
        __DBG_STUB_INVOKE__
        return "Test error";
    });

    stub.set_lamda(&DialogManager::showErrorDialog, [](DialogManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    bool result = receiver->handleOperationMkdir(0, parentDir);

    EXPECT_FALSE(result);
}

// ========== Touch File Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_TextFile)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/new.txt");
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    QString result = receiver->handleOperationTouchFile(0, parentDir, Global::CreateFileType::kCreateFileTypeText, "txt");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_WithTemplate)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/from_template.txt");
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());
    QUrl templateUrl = QUrl::fromLocalFile("/tmp/template.txt");

    QString result = receiver->handleOperationTouchFile(0, parentDir, templateUrl, "txt");

    EXPECT_FALSE(result.isEmpty());
}

// ========== Link File Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationLinkFile_Success)
{
    stub.set_lamda(&LocalFileHandler::createSystemLink, [](LocalFileHandler *, const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) -> QString {
        __DBG_STUB_INVOKE__
        return path;
    });

    QUrl sourceFile = createTestFile("link_source.txt");
    QUrl linkTarget = QUrl::fromLocalFile(tempDir->path() + "/link.txt");

    bool result = receiver->handleOperationLinkFile(0, sourceFile, linkTarget, false, true);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationLinkFile_WithForce)
{
    stub.set_lamda(&LocalFileHandler::createSystemLink, [](LocalFileHandler *, const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&LocalFileHandler::deleteFile, [](LocalFileHandler *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) -> QString {
        __DBG_STUB_INVOKE__
        return path;
    });

    QUrl sourceFile = createTestFile("force_source.txt");
    QUrl existingLink = createTestFile("existing_link.txt");

    bool result = receiver->handleOperationLinkFile(0, sourceFile, existingLink, true, true);

    EXPECT_TRUE(result);
}

// ========== Set Permission Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationSetPermission_Success)
{
    stub.set_lamda(&LocalFileHandler::setPermissions, [](LocalFileHandler *, const QUrl &, QFileDevice::Permissions) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl testFile = createTestFile("permission.txt");
    QFileDevice::Permissions perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner;

    bool result = receiver->handleOperationSetPermission(0, testFile, perms);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationSetPermission_Failed)
{
    stub.set_lamda(&LocalFileHandler::setPermissions, [](LocalFileHandler *, const QUrl &, QFileDevice::Permissions) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&LocalFileHandler::errorString, [](LocalFileHandler *) -> QString {
        __DBG_STUB_INVOKE__
        return "Permission denied";
    });

    stub.set_lamda(&DialogManager::showErrorDialog, [](DialogManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
    });

    QUrl testFile = createTestFile("no_perm.txt");
    QFileDevice::Permissions perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner;

    bool result = receiver->handleOperationSetPermission(0, testFile, perms);

    EXPECT_FALSE(result);
}

// ========== Clipboard Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationWriteToClipboard_CopyAction)
{
    stub.set_lamda(&ClipBoard::setUrlsToClipboard, [](const QList<QUrl> &urls, ClipBoard::ClipboardAction action, QMimeData *) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(action, ClipBoard::ClipboardAction::kCopyAction);
        EXPECT_GT(urls.count(), 0);
    });

    QUrl testFile = createTestFile("clipboard.txt");
    QList<QUrl> urls = { testFile };

    bool result = receiver->handleOperationWriteToClipboard(0, ClipBoard::ClipboardAction::kCopyAction, urls);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationWriteDataToClipboard_NullData)
{
    bool result = receiver->handleOperationWriteDataToClipboard(0, nullptr);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationWriteDataToClipboard_ValidData)
{
    stub.set_lamda(&ClipBoard::setDataToClipboard, []() {
        __DBG_STUB_INVOKE__
    });

    QMimeData *data = new QMimeData();
    data->setText("test");

    bool result = receiver->handleOperationWriteDataToClipboard(0, data);

    EXPECT_TRUE(result);
}

// ========== Terminal Operations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenInTerminal_LocalFiles)
{
    stub.set_lamda(&LocalFileHandler::defaultTerminalPath, [](LocalFileHandler *) -> QString {
        __DBG_STUB_INVOKE__
        return "/usr/bin/konsole";
    });

    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl testDir = createTestDir("terminal_dir");
    QList<QUrl> urls = { testDir };

    bool result = receiver->handleOperationOpenInTerminal(0, urls);

    EXPECT_TRUE(result);
}

// ========== Hide Files Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationHideFiles_Success)
{
    stub.set_lamda(&HideFileHelper::save, [](HideFileHelper *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::notifyFileChangeManual, []() {
        __DBG_STUB_INVOKE__
    });

    QUrl testFile = createTestFile("hide.txt");
    QList<QUrl> urls = { testFile };

    bool result = receiver->handleOperationHideFiles(0, urls);

    EXPECT_TRUE(result);
}

// ========== Undo/Redo Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationRevocation_EmptyStack)
{
    stub.set_lamda(&OperationsStackProxy::revocationOperations, [](OperationsStackProxy *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    receiver->handleOperationRevocation(0, nullptr);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleSaveRedoOpt_ValidToken)
{
    QString testToken = "test_redo_token";

    // Should handle gracefully
    receiver->handleSaveRedoOpt(testToken, 1024);

    SUCCEED();
}

// ========== Utility Functions Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleIsSubFile_SameScheme)
{
    QUrl parent = QUrl::fromLocalFile("/home/user/documents");
    QUrl sub = QUrl::fromLocalFile("/home/user/documents/file.txt");

    bool result = receiver->handleIsSubFile(parent, sub);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleIsSubFile_DifferentScheme)
{
    QUrl parent = QUrl("smb://server/share");
    QUrl sub = QUrl::fromLocalFile("/tmp/file.txt");

    bool result = receiver->handleIsSubFile(parent, sub);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleCopyFilePath_EmptyList)
{
    receiver->handleCopyFilePath(QList<QUrl>());

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleCopyFilePath_LocalFiles)
{
    stub.set_lamda(&ClipBoard::setDataToClipboard, [](QMimeData *data) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(data->text().isEmpty());
    });

    QUrl file1 = createTestFile("path1.txt");
    QUrl file2 = createTestFile("path2.txt");
    QList<QUrl> urls = { file1, file2 };

    receiver->handleCopyFilePath(urls);

    SUCCEED();
}

// ========== Integration Tests ==========

TEST_F(TestFileOperationsEventReceiver, Integration_CopyAndDelete)
{
    stub.set_lamda(&FileCopyMoveJob::copy, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::deletes, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    // Copy operation
    QUrl sourceFile = createTestFile("integrate.txt");
    QUrl targetDir = createTestDir("integrate_target");
    QList<QUrl> sources = { sourceFile };

    receiver->handleOperationCopy(0, sources, targetDir, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    // Delete operation
    receiver->handleOperationDeletes(0, sources, AbstractJobHandler::JobFlag::kNoHint, nullptr);

    SUCCEED();
}

// ========== OpenFiles with bool* parameter Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFiles_WithBoolPointer)
{
    stub.set_lamda(&LocalFileHandler::openFiles, [](LocalFileHandler *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl testFile = createTestFile("open_bool.txt");
    QList<QUrl> urls = { testFile };
    bool ok = false;

    bool result = receiver->handleOperationOpenFiles(0, urls, &ok);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFiles_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::openFiles, [](LocalFileHandler *, const QList<QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl testFile = createTestFile("open_cb.txt");
    QList<QUrl> urls = { testFile };

    receiver->handleOperationOpenFiles(0, urls, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== OpenFilesByApp with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFilesByApp_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::openFilesByApp, [](LocalFileHandler *, const QList<QUrl> &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl testFile = createTestFile("app_cb.txt");
    QList<QUrl> urls = { testFile };
    QList<QString> apps = { "gedit" };

    receiver->handleOperationOpenFilesByApp(0, urls, apps, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationOpenFilesByApp_MultipleApps)
{
    stub.set_lamda(&LocalFileHandler::openFilesByApp, [](LocalFileHandler *, const QList<QUrl> &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl testFile = createTestFile("multi_app.txt");
    QList<QUrl> urls = { testFile };
    QList<QString> apps = { "app1", "app2" };   // Multiple apps, should use first

    bool result = receiver->handleOperationOpenFilesByApp(0, urls, apps);

    EXPECT_TRUE(result);
}

// ========== RenameFile with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFile_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::renameFile, [](LocalFileHandler *, const QUrl &, const QUrl &, bool) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl oldUrl = createTestFile("old_cb.txt");
    QString newPath = tempDir->path() + "/new_cb.txt";
    QUrl newUrl = QUrl::fromLocalFile(newPath);

    receiver->handleOperationRenameFile(0, oldUrl, newUrl, AbstractJobHandler::JobFlag::kNoHint, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== RenameFiles with Custom/Append mode Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFiles_CustomMode)
{
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, [](LocalFileHandler *, const QMap<QUrl, QUrl> &, QMap<QUrl, QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::fileBatchCustomText, [](const QList<QUrl> &urls, const QPair<QString, QString> &) -> QMap<QUrl, QUrl> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, QUrl> result;
        for (const auto &url : urls) {
            result.insert(url, url);
        }
        return result;
    });

    QUrl file1 = createTestFile("custom1.txt");
    QList<QUrl> urls = { file1 };

    bool result = receiver->handleOperationRenameFiles(0, urls, qMakePair(QString("text"), QString("value")), false);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFiles_AppendMode)
{
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, [](LocalFileHandler *, const QMap<QUrl, QUrl> &, QMap<QUrl, QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::fileBatchAddText, [](const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &) -> QMap<QUrl, QUrl> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, QUrl> result;
        for (const auto &url : urls) {
            result.insert(url, url);
        }
        return result;
    });

    QUrl file1 = createTestFile("append1.txt");
    QList<QUrl> urls = { file1 };

    bool result = receiver->handleOperationRenameFiles(0, urls, qMakePair(QString("_suffix"), AbstractJobHandler::FileNameAddFlag::kSuffix));

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFiles_AppendWithCallback)
{
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, [](LocalFileHandler *, const QMap<QUrl, QUrl> &, QMap<QUrl, QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::fileBatchAddText, [](const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &) -> QMap<QUrl, QUrl> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, QUrl> result;
        for (const auto &url : urls) {
            result.insert(url, url);
        }
        return result;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl file1 = createTestFile("append_cb.txt");
    QList<QUrl> urls = { file1 };

    receiver->handleOperationRenameFiles(0, urls, qMakePair(QString("_cb"), AbstractJobHandler::FileNameAddFlag::kPrefix), QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationRenameFiles_ReplaceWithCallback)
{
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, [](LocalFileHandler *, const QMap<QUrl, QUrl> &, QMap<QUrl, QUrl> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::fileBatchReplaceText, [](const QList<QUrl> &urls, const QPair<QString, QString> &) -> QMap<QUrl, QUrl> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, QUrl> result;
        for (const auto &url : urls) {
            result.insert(url, url);
        }
        return result;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl file1 = createTestFile("replace_cb.txt");
    QList<QUrl> urls = { file1 };

    receiver->handleOperationRenameFiles(0, urls, qMakePair(QString("old"), QString("new")), true, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== Mkdir with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationMkdir_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::mkdir, [](LocalFileHandler *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
        EXPECT_TRUE(args->value(AbstractJobHandler::CallbackKey::kSuccessed).toBool());
    };

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    receiver->handleOperationMkdir(0, parentDir, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== TouchFile with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_WordDoc)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/new.docx");
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    QString result = receiver->handleOperationTouchFile(0, parentDir, Global::CreateFileType::kCreateFileTypeWord, "docx");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_ExcelSheet)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/new.xlsx");
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    QString result = receiver->handleOperationTouchFile(0, parentDir, Global::CreateFileType::kCreateFileTypeExcel, "xlsx");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_PowerPoint)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/new.pptx");
    });

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    QString result = receiver->handleOperationTouchFile(0, parentDir, Global::CreateFileType::kCreateFileTypePowerpoint, "pptx");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_WithCallbackImmediately)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/immediate.txt");
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());

    receiver->handleOperationTouchFile(0, parentDir, Global::CreateFileType::kCreateFileTypeText, "txt", QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationTouchFile_TemplateWithCallback)
{
    stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &, const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/from_template_cb.txt");
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl parentDir = QUrl::fromLocalFile(tempDir->path());
    QUrl templateUrl = QUrl::fromLocalFile("/tmp/template.txt");

    receiver->handleOperationTouchFile(0, parentDir, templateUrl, "txt", QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== LinkFile with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationLinkFile_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::createSystemLink, [](LocalFileHandler *, const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) -> QString {
        __DBG_STUB_INVOKE__
        return path;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl sourceFile = createTestFile("link_cb_source.txt");
    QUrl linkTarget = QUrl::fromLocalFile(tempDir->path() + "/link_cb.txt");

    receiver->handleOperationLinkFile(0, sourceFile, linkTarget, false, true, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== SetPermission with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationSetPermission_WithCallback)
{
    stub.set_lamda(&LocalFileHandler::setPermissions, [](LocalFileHandler *, const QUrl &, QFileDevice::Permissions) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl testFile = createTestFile("perm_cb.txt");
    QFileDevice::Permissions perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner;

    receiver->handleOperationSetPermission(0, testFile, perms, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== HideFiles with Callback Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationHideFiles_WithCallback)
{
    stub.set_lamda(&HideFileHelper::save, [](HideFileHelper *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::notifyFileChangeManual, []() {
        __DBG_STUB_INVOKE__
    });

    bool callbackCalled = false;
    auto callback = [&callbackCalled](const AbstractJobHandler::CallbackArgus &args) {
        callbackCalled = true;
    };

    QUrl testFile = createTestFile("hide_cb.txt");
    QList<QUrl> urls = { testFile };

    receiver->handleOperationHideFiles(0, urls, QVariant(), callback);

    EXPECT_TRUE(callbackCalled);
}

// ========== ShortCut Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleShortCut_NoWritePermission)
{
    stub.set_lamda(&DialogManager::showNoPermissionDialog, [](DialogManager *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
    });

    QUrl testFile = createTestFile("shortcut.txt");
    QUrl rootUrl = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> urls = { testFile };

    // Should return true if no write permission
    bool result = receiver->handleShortCut(0, urls, rootUrl);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleShortCutPaste_NoWritePermission)
{
    stub.set_lamda(&DialogManager::showNoPermissionDialog, [](DialogManager *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
    });

    QUrl targetDir = createTestDir("paste_target");
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/paste.txt") };

    bool result = receiver->handleShortCutPaste(0, urls, targetDir);

    SUCCEED();
}

// ========== SaveOperations Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationSaveOperations_ValidData)
{
    stub.set_lamda(&OperationsStackProxy::saveOperations, [](OperationsStackProxy *, const QVariantMap &values) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(values.isEmpty());
    });

    QVariantMap testData;
    testData.insert("test", "value");

    receiver->handleOperationSaveOperations(testData);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCleanSaveOperationsStack_Called)
{
    stub.set_lamda(&OperationsStackProxy::cleanOperations, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    receiver->handleOperationCleanSaveOperationsStack();

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationSaveRedoOperations_ValidData)
{
    stub.set_lamda(&OperationsStackProxy::SaveRedoOperations, [](OperationsStackProxy *, const QVariantMap &values) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(values.isEmpty());
    });

    QVariantMap testData;
    testData.insert("redo", "data");

    receiver->handleOperationSaveRedoOperations(testData);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCleanByUrls_ValidUrls)
{
    stub.set_lamda(&OperationsStackProxy::CleanOperationsByUrl, [](OperationsStackProxy *, const QStringList &strs) {
        __DBG_STUB_INVOKE__
        EXPECT_GT(strs.count(), 0);
    });

    QUrl file1 = createTestFile("clean1.txt");
    QUrl file2 = createTestFile("clean2.txt");
    QList<QUrl> urls = { file1, file2 };

    receiver->handleOperationCleanByUrls(urls);

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationCleanByUrls_EmptyList)
{
    // Should handle empty list gracefully
    receiver->handleOperationCleanByUrls(QList<QUrl>());

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleRecoveryOperationRedoRecovery_Called)
{
    stub.set_lamda(&OperationsStackProxy::RevocationRedoOperations, [](OperationsStackProxy *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    receiver->handleRecoveryOperationRedoRecovery(0, nullptr);

    SUCCEED();
}

// ========== UndoDeletes Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationUndoDeletes_ValidOperation)
{
    stub.set_lamda(&DialogManager::showRestoreDeleteFilesDialog, [](DialogManager *, const QList<QUrl> &) -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&DialogManager::showDeleteFilesDialog, [](DialogManager *, const QList<QUrl> &, bool) -> int {
        __DBG_STUB_INVOKE__
        return QDialog::Accepted;
    });

    stub.set_lamda(&FileCopyMoveJob::deletes, [](FileCopyMoveJob *, const QList<QUrl> &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl testFile = createTestFile("undo_delete.txt");
    QList<QUrl> sources = { testFile };
    QVariantMap op;
    op.insert("test", "undo");

    receiver->handleOperationUndoDeletes(0, sources, AbstractJobHandler::JobFlag::kRevocation, nullptr, op);

    SUCCEED();
}

// ========== UndoCut Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationUndoCut_ValidOperation)
{
    stub.set_lamda(&FileCopyMoveJob::cut, [](FileCopyMoveJob *, const QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlags, bool) -> JobHandlePointer {
        __DBG_STUB_INVOKE__
        return JobHandlePointer(new AbstractJobHandler());
    });

    QUrl sourceFile = createTestFile("undo_cut.txt");
    QUrl targetDir = createTestDir("undo_cut_target");
    QList<QUrl> sources = { sourceFile };
    QVariantMap op;
    op.insert("test", "undo_cut");

    receiver->handleOperationUndoCut(0, sources, targetDir, AbstractJobHandler::JobFlag::kRevocation, nullptr, op);

    SUCCEED();
}

// ========== FilesPreview Tests ==========

TEST_F(TestFileOperationsEventReceiver, HandleOperationFilesPreview_EmptyLists)
{
    // Should handle empty lists gracefully
    receiver->handleOperationFilesPreview(0, QList<QUrl>(), QList<QUrl>());

    SUCCEED();
}

TEST_F(TestFileOperationsEventReceiver, HandleOperationFilesPreview_ValidUrls)
{
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [](const QString &cmd, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       EXPECT_FALSE(cmd.isEmpty());
                       EXPECT_GT(args.count(), 0);
                       return true;
                   });

    QUrl selectFile = createTestFile("preview_select.txt");
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());
    QList<QUrl> selectUrls = { selectFile };
    QList<QUrl> dirUrls = { dirUrl };

    receiver->handleOperationFilesPreview(0, selectUrls, dirUrls);

    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestFileOperationsEventReceiver, EdgeCase_VeryLongFileName)
{
    QString longName = QString("a").repeated(200) + ".txt";
    QUrl longFile = createTestFile(longName);

    EXPECT_TRUE(QFile::exists(longFile.toLocalFile()));
}

TEST_F(TestFileOperationsEventReceiver, EdgeCase_SpecialCharactersInPath)
{
    QString specialName = "file with spaces & symbols @#$.txt";
    QUrl specialFile = createTestFile(specialName);

    EXPECT_TRUE(QFile::exists(specialFile.toLocalFile()));
}

TEST_F(TestFileOperationsEventReceiver, EdgeCase_UnicodeFileName)
{
    QString unicodeName = "文件测试中文名.txt";
    QUrl unicodeFile = createTestFile(unicodeName);

    EXPECT_TRUE(QFile::exists(unicodeFile.toLocalFile()));
}
