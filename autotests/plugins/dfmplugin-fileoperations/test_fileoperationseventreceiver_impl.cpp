// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QMimeData>
#include <QClipboard>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperationsevent/fileoperationseventhandler.h"
#include "fileoperations/fileoperationsservice.h"
#include "fileoperations/filecopymovejob.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class FileOpsEventReceiverImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        receiver = FileOperationsEventReceiver::instance();
        ASSERT_TRUE(receiver);

        // Never block on modal error dialogs (QDialog::exec() would hang the
        // whole test run offscreen); let the operation result speak instead.
        stub.set_lamda(&dfmbase::DialogManager::showErrorDialog,
                       [](dfmbase::DialogManager *, const QString &, const QString &) {
                           __DBG_STUB_INVOKE__
                       });

        // Confirm dialogs must return Accepted so the tested code path
        // actually runs; failure/info popups become no-ops.
        stub.set_lamda(&dfmbase::DialogManager::showDeleteFilesDialog,
                       [](dfmbase::DialogManager *, const QList<QUrl> &, bool) -> int {
                           __DBG_STUB_INVOKE__
                           return QDialog::Accepted;
                       });
        stub.set_lamda(&dfmbase::DialogManager::showRestoreDeleteFilesDialog,
                       [](dfmbase::DialogManager *, const QList<QUrl> &) -> int {
                           __DBG_STUB_INVOKE__
                           return QDialog::Accepted;
                       });
        stub.set_lamda(&dfmbase::DialogManager::showNormalDeleteConfirmDialog,
                       [](dfmbase::DialogManager *, const QList<QUrl> &) -> int {
                           __DBG_STUB_INVOKE__
                           return QDialog::Accepted;
                       });
        stub.set_lamda(&dfmbase::DialogManager::showNoPermissionDialog,
                       [](dfmbase::DialogManager *, const QList<QUrl> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&dfmbase::DialogManager::showRestoreFailedDialog,
                       [](dfmbase::DialogManager *, int) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&dfmbase::DialogManager::showOperationFailedDialog,
                       [](dfmbase::DialogManager *, const QMap<QUrl, QString> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&dfmbase::DialogManager::showCopyMoveToSelfDialog,
                       [](dfmbase::DialogManager *) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&dfmbase::DialogManager::showRenameBusyErrDialog,
                       [](dfmbase::DialogManager *) {
                           __DBG_STUB_INVOKE__
                       });

        // Clipboard: handing QMimeData to the real offscreen clipboard across
        // suites leaves dangling ownership (some cases delete the data after
        // handing it over), which crashes the next setMimeData call. Intercept
        // all clipboard writes so no test data ever reaches it.
        stub.set_lamda(&dfmbase::ClipBoard::setDataToClipboard,
                       [](QMimeData *data) {
                           __DBG_STUB_INVOKE__
                           delete data;
                       });
        stub.set_lamda(&dfmbase::ClipBoard::setUrlsToClipboard,
                       [](const QList<QUrl> &, dfmbase::ClipBoard::ClipboardAction, QMimeData *data) {
                           __DBG_STUB_INVOKE__
                           delete data;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl createTestFile(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("test");
            file.close();
        }
        return QUrl::fromLocalFile(path);
    }

    QUrl createTestDir(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QDir().mkpath(path);
        return QUrl::fromLocalFile(path);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    FileOperationsEventReceiver *receiver { nullptr };
};

// ========== newDocmentName ==========
TEST_F(FileOpsEventReceiverImpl, NewDocmentName_Folder)
{
    QUrl dir = createTestDir("newfolder");
    QString name = receiver->newDocmentName(dir, QString(), CreateFileType::kCreateFileTypeFolder);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("New Folder") || name.contains("NewFolder") || name.contains(QObject::tr("New Folder")));
}

TEST_F(FileOpsEventReceiverImpl, NewDocmentName_Text)
{
    QUrl dir = createTestDir("newtext");
    QString name = receiver->newDocmentName(dir, QString(), CreateFileType::kCreateFileTypeText);
    EXPECT_FALSE(name.isEmpty());
}

TEST_F(FileOpsEventReceiverImpl, NewDocmentName_InvalidUrl)
{
    QString name = receiver->newDocmentName(QUrl(), "txt", CreateFileType::kCreateFileTypeDefault);
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(FileOpsEventReceiverImpl, NewDocmentName_ByBaseName)
{
    QUrl dir = createTestDir("bybasename");
    QString name = receiver->newDocmentName(dir, "MyBase", "txt");
    EXPECT_FALSE(name.isEmpty());
}

// ========== handleOperationCopy / Cut / Deletes overloads ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationCopy_Basic)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doCopyFile),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationCopy(1, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"),
                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationCopy_WithCallback)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doCopyFile),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationCopy(1, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"),
                                  AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationCut_Basic)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doCutFile),
                   [](FileOperationsEventReceiver *, quint64, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback, const bool) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationCut(1, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"),
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationCut_WithCallback)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doCutFile),
                   [](FileOperationsEventReceiver *, quint64, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback, const bool) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationCut(1, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"),
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationDeletes_Basic)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doDeleteFile),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback,
                      const bool, FileOperationsEventReceiver::DoDeleteErrorType &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationDeletes(1, { QUrl::fromLocalFile("/tmp/a") },
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationDeletes_WithCallback)
{
    stub.set_lamda(VADDR(FileOperationsEventReceiver, doDeleteFile),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &,
                      const AbstractJobHandler::JobFlags &, AbstractJobHandler::OperatorHandleCallback,
                      const bool, FileOperationsEventReceiver::DoDeleteErrorType &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->handleOperationDeletes(1, { QUrl::fromLocalFile("/tmp/a") },
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariant(), nullptr);
    SUCCEED();
}

// ========== Open files ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenFiles_Basic)
{
    QUrl file = createTestFile("open.txt");
    bool result = receiver->handleOperationOpenFiles(1, { file });
    // Result depends on environment; ensure no crash
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenFiles_WithOk)
{
    QUrl file = createTestFile("open2.txt");
    bool ok = false;
    bool result = receiver->handleOperationOpenFiles(1, { file }, &ok);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenFiles_Callback)
{
    QUrl file = createTestFile("open3.txt");
    receiver->handleOperationOpenFiles(1, { file }, QVariant(), [](const AbstractJobHandler::CallbackArgus &) {
        __DBG_STUB_INVOKE__
    });
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenFilesByApp_Basic)
{
    QUrl file = createTestFile("openapp.txt");
    bool result = receiver->handleOperationOpenFilesByApp(1, { file }, { "deepin-editor" });
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenFilesByApp_Callback)
{
    QUrl file = createTestFile("openapp2.txt");
    receiver->handleOperationOpenFilesByApp(1, { file }, { "deepin-editor" }, QVariant(), nullptr);
    SUCCEED();
}

// ========== Rename operations ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFile_Basic)
{
    QUrl oldUrl = createTestFile("old.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/new.txt");

    stub.set_lamda(ADDR(FileOperationsEventReceiver, doRenameDesktopFile),
                   [](FileOperationsEventReceiver *, const quint64, const QUrl, const QUrl,
                      const AbstractJobHandler::JobFlags) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleOperationRenameFile(1, oldUrl, newUrl,
                                                       AbstractJobHandler::JobFlag::kNoHint);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFile_Callback)
{
    QUrl oldUrl = createTestFile("old2.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/new2.txt");

    receiver->handleOperationRenameFile(1, oldUrl, newUrl,
                                       AbstractJobHandler::JobFlag::kNoHint, QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFiles_BasicReplace)
{
    QUrl file = createTestFile("rename.txt");
    QPair<QString, QString> pair("a", "b");

    stub.set_lamda(ADDR(FileOperationsEventReceiver, doRenameFiles),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &,
                      const QPair<QString, QString> &, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &,
                      FileOperationsEventReceiver::RenameTypes, QMap<QUrl, QUrl> &, QString &,
                      const QVariant, AbstractJobHandler::OperatorCallback) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleOperationRenameFiles(1, { file }, pair, true);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFiles_CallbackReplace)
{
    QUrl file = createTestFile("rename2.txt");
    QPair<QString, QString> pair("a", "b");

    receiver->handleOperationRenameFiles(1, { file }, pair, true, QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFiles_BasicAppend)
{
    QUrl file = createTestFile("rename3.txt");
    QPair<QString, AbstractJobHandler::FileNameAddFlag> pair("new", AbstractJobHandler::FileNameAddFlag::kPrefix);

    stub.set_lamda(ADDR(FileOperationsEventReceiver, doRenameFiles),
                   [](FileOperationsEventReceiver *, const quint64, const QList<QUrl> &,
                      const QPair<QString, QString> &, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &,
                      FileOperationsEventReceiver::RenameTypes, QMap<QUrl, QUrl> &, QString &,
                      const QVariant, AbstractJobHandler::OperatorCallback) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleOperationRenameFiles(1, { file }, pair);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRenameFiles_CallbackAppend)
{
    QUrl file = createTestFile("rename4.txt");
    QPair<QString, AbstractJobHandler::FileNameAddFlag> pair("new", AbstractJobHandler::FileNameAddFlag::kPrefix);

    receiver->handleOperationRenameFiles(1, { file }, pair, QVariant(), nullptr);
    SUCCEED();
}

// ========== Mkdir ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationMkdir_Basic)
{
    QUrl dir = QUrl::fromLocalFile(tempDir->path() + "/mkdir_basic");

    stub.set_lamda(ADDR(FileOperationsEventReceiver, doMkdir),
                   [](FileOperationsEventReceiver *, const quint64, const QUrl &, const QVariant &,
                      AbstractJobHandler::OperatorCallback, const bool) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleOperationMkdir(1, dir);
    EXPECT_TRUE(result);
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationMkdir_Callback)
{
    QUrl dir = QUrl::fromLocalFile(tempDir->path() + "/mkdir_callback");

    stub.set_lamda(ADDR(FileOperationsEventReceiver, doMkdir),
                   [](FileOperationsEventReceiver *, const quint64, const QUrl &, const QVariant &,
                      AbstractJobHandler::OperatorCallback, const bool) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    receiver->handleOperationMkdir(1, dir, QVariant(), nullptr);
    SUCCEED();
}

// ========== Touch file ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationTouchFile_ByType)
{
    QUrl dir = createTestDir("touchdir");
    QUrl url = QUrl::fromLocalFile(dir.path() + "/touch.txt");

    stub.set_lamda(
            static_cast<QString (FileOperationsEventReceiver::*)(const quint64, const QUrl &, const CreateFileType,
                                                                 const QString &, const QVariant &,
                                                                 AbstractJobHandler::OperatorCallback)>(
                    &FileOperationsEventReceiver::doTouchFilePremature),
            [](FileOperationsEventReceiver *, const quint64, const QUrl &, const CreateFileType,
               const QString &, const QVariant &, AbstractJobHandler::OperatorCallback) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("/tmp/stub.txt");
                   });

    QString result = receiver->handleOperationTouchFile(1, url, CreateFileType::kCreateFileTypeText, "txt");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationTouchFile_ByTempUrl)
{
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/touch2.txt");
    QUrl tempUrl = createTestFile("template.txt");

    stub.set_lamda(
            static_cast<QString (FileOperationsEventReceiver::*)(const quint64, const QUrl &, const QUrl &,
                                                                 const QString &, const QVariant &,
                                                                 AbstractJobHandler::OperatorCallback)>(
                    &FileOperationsEventReceiver::doTouchFilePremature),
            [](FileOperationsEventReceiver *, const quint64, const QUrl &, const QUrl &,
               const QString &, const QVariant &, AbstractJobHandler::OperatorCallback) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("/tmp/stub.txt");
                   });

    QString result = receiver->handleOperationTouchFile(1, url, tempUrl, "txt");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationTouchFile_CallbackByType)
{
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/touch3.txt");

    stub.set_lamda(
            static_cast<QString (FileOperationsEventReceiver::*)(const quint64, const QUrl &, const CreateFileType,
                                                                 const QString &, const QVariant &,
                                                                 AbstractJobHandler::OperatorCallback)>(
                    &FileOperationsEventReceiver::doTouchFilePremature),
            [](FileOperationsEventReceiver *, const quint64, const QUrl &, const CreateFileType,
               const QString &, const QVariant &, AbstractJobHandler::OperatorCallback) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("/tmp/stub.txt");
                   });

    receiver->handleOperationTouchFile(1, url, CreateFileType::kCreateFileTypeText, "txt", QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationTouchFile_CallbackByTempUrl)
{
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/touch4.txt");
    QUrl tempUrl = createTestFile("template2.txt");

    stub.set_lamda(
            static_cast<QString (FileOperationsEventReceiver::*)(const quint64, const QUrl &, const QUrl &,
                                                                 const QString &, const QVariant &,
                                                                 AbstractJobHandler::OperatorCallback)>(
                    &FileOperationsEventReceiver::doTouchFilePremature),
            [](FileOperationsEventReceiver *, const quint64, const QUrl &, const QUrl &,
               const QString &, const QVariant &, AbstractJobHandler::OperatorCallback) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("/tmp/stub.txt");
                   });

    receiver->handleOperationTouchFile(1, url, tempUrl, "txt", QVariant(), nullptr);
    SUCCEED();
}

// ========== Link file ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationLinkFile_Basic)
{
    QUrl source = createTestFile("link_source.txt");
    QUrl link = QUrl::fromLocalFile(tempDir->path() + "/link_basic.lnk");

    stub.set_lamda(ADDR(FileOperationsEventReceiver, determineLinkTarget),
                   [](FileOperationsEventReceiver *, const QUrl &, const QUrl &, const bool, const quint64) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile("/tmp/stub_target");
                   });

    bool result = receiver->handleOperationLinkFile(1, source, link, false, true);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationLinkFile_Callback)
{
    QUrl source = createTestFile("link_source2.txt");
    QUrl link = QUrl::fromLocalFile(tempDir->path() + "/link_callback.lnk");

    receiver->handleOperationLinkFile(1, source, link, false, true, QVariant(), nullptr);
    SUCCEED();
}

// ========== Permission ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationSetPermission_Basic)
{
    QUrl file = createTestFile("perm.txt");

    stub.set_lamda(ADDR(LocalFileHandler, setPermissions),
                   [](LocalFileHandler *, const QUrl &, const QFileDevice::Permissions) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleOperationSetPermission(1, file, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationSetPermission_Callback)
{
    QUrl file = createTestFile("perm2.txt");

    stub.set_lamda(ADDR(LocalFileHandler, setPermissions),
                   [](LocalFileHandler *, const QUrl &, const QFileDevice::Permissions) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    receiver->handleOperationSetPermission(1, file, QFileDevice::ReadOwner, QVariant(), nullptr);
    SUCCEED();
}

// ========== Clipboard operations ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationWriteToClipboard)
{
    QUrl file = createTestFile("clipboard.txt");
    bool result = receiver->handleOperationWriteToClipboard(1, ClipBoard::kCopyAction, { file });
    // Depends on clipboard state; ensure no crash
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationWriteDataToClipboard)
{
    // The fixture stub intercepts setDataToClipboard and deletes the data
    // itself (never hand QMimeData to the real clipboard here), so no extra
    // delete below.
    QMimeData *data = new QMimeData();
    data->setText("test data");
    bool result = receiver->handleOperationWriteDataToClipboard(1, data);
    SUCCEED();
}

// ========== Open in terminal ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationOpenInTerminal)
{
    QUrl dir = createTestDir("terminal");
    bool result = receiver->handleOperationOpenInTerminal(1, { dir });
    SUCCEED();
}

// ========== Save / clean operations stack ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationSaveOperations)
{
    QVariantMap values;
    values.insert("test", "value");
    receiver->handleOperationSaveOperations(values);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationCleanSaveOperationsStack)
{
    receiver->handleOperationCleanSaveOperationsStack();
    SUCCEED();
}

// ========== Hide files ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationHideFiles_Basic)
{
    QUrl file = createTestFile("hide.txt");
    bool result = receiver->handleOperationHideFiles(1, { file });
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationHideFiles_Callback)
{
    QUrl file = createTestFile("hide2.txt");
    receiver->handleOperationHideFiles(1, { file }, QVariant(), nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationHideFiles_ByParent)
{
    QUrl parent = createTestDir("hideparent");
    QUrl file = createTestFile("hideparent/hide3.txt");
    bool result = receiver->handleOperationHideFiles(1, parent, { file }, true);
    SUCCEED();
}

// ========== ShortCut ==========
TEST_F(FileOpsEventReceiverImpl, HandleShortCut)
{
    QUrl root = createTestDir("shortcutroot");
    QUrl file = createTestFile("shortcutroot/file.txt");
    bool result = receiver->handleShortCut(1, { file }, root);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleShortCutPaste)
{
    QUrl target = createTestDir("shortcuttarget");
    bool result = receiver->handleShortCutPaste(1, { QUrl::fromLocalFile("/tmp/a") }, target);
    SUCCEED();
}

// ========== Redo / undo / save redo ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationSaveRedoOperations)
{
    QVariantMap values;
    values.insert("undoevent", static_cast<uint16_t>(GlobalEventType::kCopy));
    values.insert("undosources", QStringList({ "/tmp/a" }));
    values.insert("undotargets", QStringList({ "/tmp/b" }));

    receiver->handleOperationSaveRedoOperations(values);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationCleanByUrls)
{
    receiver->handleOperationCleanByUrls({ QUrl::fromLocalFile("/tmp/a") });
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationUndoDeletes)
{
    receiver->handleOperationUndoDeletes(1, { QUrl::fromLocalFile("/tmp/a") },
                                       AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariantMap());
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationUndoCut)
{
    receiver->handleOperationUndoCut(1, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"),
                                   AbstractJobHandler::JobFlag::kNoHint, nullptr, QVariantMap());
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleSaveRedoOpt)
{
    receiver->handleSaveRedoOpt("token", 1024);
    SUCCEED();
}

// ========== IsSubFile ==========
TEST_F(FileOpsEventReceiverImpl, HandleIsSubFile)
{
    QUrl parent = QUrl::fromLocalFile("/tmp/parent");
    QUrl child = QUrl::fromLocalFile("/tmp/parent/child.txt");
    bool result = receiver->handleIsSubFile(parent, child);
    EXPECT_TRUE(result);
}

// ========== CopyFilePath ==========
TEST_F(FileOpsEventReceiverImpl, HandleCopyFilePath)
{
    receiver->handleCopyFilePath({ QUrl::fromLocalFile("/tmp/a.txt") });
    SUCCEED();
}

// ========== Files preview ==========
TEST_F(FileOpsEventReceiverImpl, HandleOperationFilesPreview)
{
    receiver->handleOperationFilesPreview(1, { QUrl::fromLocalFile("/tmp/a.txt") },
                                          { QUrl::fromLocalFile("/tmp") });
    SUCCEED();
}

// ========== Recovery / revocation ==========
TEST_F(FileOpsEventReceiverImpl, HandleRecoveryOperationRedoRecovery)
{
    receiver->handleRecoveryOperationRedoRecovery(1, nullptr);
    SUCCEED();
}

TEST_F(FileOpsEventReceiverImpl, HandleOperationRevocation)
{
    receiver->handleOperationRevocation(1, nullptr);
    SUCCEED();
}

// ========== doRenameFiles internal ==========
TEST_F(FileOpsEventReceiverImpl, DoRenameFiles_BatchReplaceEmpty)
{
    QMap<QUrl, QUrl> successUrls;
    QString errorMsg;
    QPair<QString, QString> pair("a", "b");
    QPair<QString, AbstractJobHandler::FileNameAddFlag> pair2;

    bool result = receiver->doRenameFiles(1, {}, pair, pair2,
                                          FileOperationsEventReceiver::RenameTypes::kBatchRepalce,
                                          successUrls, errorMsg, QVariant(), nullptr);
    // Empty urls produce nothing to deal: doRenameFiles returns false when
    // the replace result is empty.
    EXPECT_FALSE(result);
}

// ========== doMkdir internal ==========
TEST_F(FileOpsEventReceiverImpl, DoMkdir_ValidPath)
{
    QUrl dir = QUrl::fromLocalFile(tempDir->path() + "/do_mkdir");
    bool result = receiver->doMkdir(1, dir, QVariant(), nullptr, true);
    EXPECT_TRUE(result);
    EXPECT_TRUE(QFile::exists(dir.path()));
}

TEST_F(FileOpsEventReceiverImpl, DoMkdir_ExistingPath)
{
    QUrl dir = createTestDir("existing_mkdir");
    // The product does not treat "already exists" as success: mkdir fails.
    bool result = receiver->doMkdir(1, dir, QVariant(), nullptr, true);
    EXPECT_FALSE(result);
}

// ========== determineLinkTarget ==========
TEST_F(FileOpsEventReceiverImpl, DetermineLinkTarget_LocalFile)
{
    QUrl source = createTestFile("linktarget.txt");
    QUrl link = QUrl::fromLocalFile(tempDir->path() + "/determined_link");

    QUrl result = receiver->determineLinkTarget(source, link, true, 1);
    SUCCEED();
}

// ========== handleIsSubFile edge cases ==========
TEST_F(FileOpsEventReceiverImpl, HandleIsSubFile_SameUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/parent");
    // Implementation is a plain startsWith: an identical path counts as
    // "sub" (true).
    bool result = receiver->handleIsSubFile(url, url);
    EXPECT_TRUE(result);
}

TEST_F(FileOpsEventReceiverImpl, HandleIsSubFile_InvalidParent)
{
    QUrl child = QUrl::fromLocalFile("/tmp/parent/child.txt");
    bool result = receiver->handleIsSubFile(QUrl(), child);
    EXPECT_FALSE(result);
}
