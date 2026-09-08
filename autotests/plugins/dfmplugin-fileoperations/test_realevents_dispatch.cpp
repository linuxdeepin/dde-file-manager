// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real event-dispatch tests: wire handlers through the production
// FileOperations::initialize() subscription list (plus a manual hook follow
// for hook_Url_IsSubFile) and then PUBLISH the real GlobalEventType events
// with exactly-typed QVariant arguments, so every EventHelper<M>::invoke /
// EventHelper<M>::EventHelper template instantiation owned by
// dfmplugin_fileoperations in include/dfm-framework/event/eventhelper.h
// actually executes. Heavy handler dependencies (FileCopyMoveJob,
// DialogManager, ClipBoard, LocalFileHandler, QProcess, OperationsStackProxy)
// are stubbed and their arguments captured for exact-value assertions.

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>
#include <QDialog>
#include <QApplication>
#include <QImage>
#include <QClipboard>

#include "stubext.h"
#include "dfm_hookreg.h"

#include "fileoperations.h"
#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperationsevent/trashfileeventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include "fileoperations/operationsstackproxy.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/hidefilehelper.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <DDesktopServices>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

namespace {

// ---------- captured arguments / call flags (reset in SetUp) ----------
bool g_copyCalled = false;
bool g_cutCalled = false;
bool g_deletesCalled = false;
bool g_moveToTrashCalled = false;
bool g_restoreFromTrashCalled = false;
bool g_copyFromTrashCalled = false;
bool g_cleanTrashCalled = false;
bool g_initArgsCalled = false;
bool g_mkdirCalled = false;
bool g_touchFileCalled = false;
bool g_renameFileCalled = false;
bool g_renameBatchCalled = false;
bool g_openFilesCalled = false;
bool g_openFilesByAppCalled = false;
bool g_setPermsCalled = false;
bool g_linkCalled = false;
bool g_clipboardUrlsCalled = false;
bool g_clipboardDataCalled = false;
bool g_replaceClipCalled = false;
bool g_hideSaveCalled = false;
bool g_saveOpsCalled = false;
bool g_saveRedoOpsCalled = false;
bool g_cleanOpsCalled = false;
bool g_cleanOpsByUrlsCalled = false;
bool g_detached2Called = false;
bool g_detached3Called = false;
bool g_soundPlayed = false;
bool g_noPermDialogCalled = false;
bool g_deleteDialogShown = false;
bool g_handleCbCalled = false;

QList<QUrl> g_lastCopySources;
QUrl g_lastCopyTarget;
QList<QUrl> g_lastCutSources;
QUrl g_lastCutTarget;
QList<QUrl> g_lastDeleteSources;
QList<QUrl> g_lastMoveTrashSources;
AbstractJobHandler::JobFlags g_lastMoveTrashFlags;
QList<QUrl> g_lastRestoreSources;
QUrl g_lastRestoreTarget;
QList<QUrl> g_lastCopyTrashSources;
QUrl g_lastCopyTrashTarget;
QList<QUrl> g_lastCleanTrashUrls;
QUrl g_lastMkdirUrl;
QUrl g_lastTouchUrl;
QUrl g_lastTouchTempUrl;
QUrl g_lastRenameOld;
QUrl g_lastRenameNew;
QMap<QUrl, QUrl> g_lastBatchMap;
QList<QUrl> g_lastOpenUrls;
QString g_lastOpenApp;
QFileDevice::Permissions g_lastPerms;
QUrl g_lastLinkSource;
QUrl g_lastLinkTarget;
ClipBoard::ClipboardAction g_lastClipAction = ClipBoard::ClipboardAction::kCopyAction;
QList<QUrl> g_lastClipUrls;
QMimeData *g_lastMimeData = nullptr;
QString g_lastClipboardText;
QStringList g_lastCleanOpsUrls;
QStringList g_lastDetached2Args;
QString g_lastDetachedProgram;
QVariantMap g_lastSavedOps;
QVariantMap g_lastSavedRedoOps;

// operator-callback capture
struct CallbackState
{
    bool called = false;
    QVariant custom;
    quint64 windowId = 0;
    bool success = false;
    bool hasHandle = false;
    QList<QUrl> sourceUrls;
};
CallbackState g_opCb;

void resetCaptures()
{
    g_copyCalled = g_cutCalled = g_deletesCalled = g_moveToTrashCalled = false;
    g_restoreFromTrashCalled = g_copyFromTrashCalled = g_cleanTrashCalled = false;
    g_initArgsCalled = g_mkdirCalled = g_touchFileCalled = g_renameFileCalled = false;
    g_renameBatchCalled = g_openFilesCalled = g_openFilesByAppCalled = false;
    g_setPermsCalled = g_linkCalled = g_clipboardUrlsCalled = g_clipboardDataCalled = false;
    g_replaceClipCalled = g_hideSaveCalled = g_saveOpsCalled = g_saveRedoOpsCalled = false;
    g_cleanOpsCalled = g_cleanOpsByUrlsCalled = g_detached2Called = g_detached3Called = false;
    g_soundPlayed = g_noPermDialogCalled = g_deleteDialogShown = g_handleCbCalled = false;

    g_lastCopySources.clear();
    g_lastCopyTarget = QUrl();
    g_lastCutSources.clear();
    g_lastCutTarget = QUrl();
    g_lastDeleteSources.clear();
    g_lastMoveTrashSources.clear();
    g_lastMoveTrashFlags = AbstractJobHandler::JobFlags(AbstractJobHandler::JobFlag::kNoHint);
    g_lastRestoreSources.clear();
    g_lastRestoreTarget = QUrl();
    g_lastCopyTrashSources.clear();
    g_lastCopyTrashTarget = QUrl();
    g_lastCleanTrashUrls.clear();
    g_lastMkdirUrl = QUrl();
    g_lastTouchUrl = QUrl();
    g_lastTouchTempUrl = QUrl();
    g_lastRenameOld = QUrl();
    g_lastRenameNew = QUrl();
    g_lastBatchMap.clear();
    g_lastOpenUrls.clear();
    g_lastOpenApp = QString();
    g_lastPerms = QFileDevice::Permissions();
    g_lastLinkSource = QUrl();
    g_lastLinkTarget = QUrl();
    g_lastClipUrls.clear();
    g_lastMimeData = nullptr;
    g_lastClipboardText = QString();
    g_lastCleanOpsUrls.clear();
    g_lastDetached2Args.clear();
    g_lastDetachedProgram = QString();
    g_lastSavedOps.clear();
    g_lastSavedRedoOps.clear();
    g_opCb = CallbackState();
}

// captureless callbacks so they can travel inside QVariant::fromValue
void takeHandleCallback(const JobHandlePointer &handle)
{
    g_handleCbCalled = handle != nullptr;
}

void takeOperatorCallback(const AbstractJobHandler::CallbackArgus &args)
{
    g_opCb.called = true;
    g_opCb.custom = args->value(AbstractJobHandler::CallbackKey::kCustom);
    g_opCb.windowId = args->value(AbstractJobHandler::CallbackKey::kWindowId).toULongLong();
    g_opCb.success = args->value(AbstractJobHandler::CallbackKey::kSuccessed).toBool();
    g_opCb.hasHandle = args->contains(AbstractJobHandler::CallbackKey::kJobHandle);
    const auto &srcs = args->value(AbstractJobHandler::CallbackKey::kSourceUrls);
    if (srcs.canConvert<QList<QUrl>>())
        g_opCb.sourceUrls = srcs.value<QList<QUrl>>();
}

// wrap the free functions into the exact std::function types the handlers
// expect, otherwise QVariant::fromValue would store a raw function pointer
QVariant makeHandleCallback()
{
    return QVariant::fromValue(AbstractJobHandler::OperatorHandleCallback(takeHandleCallback));
}

QVariant makeOperatorCallback()
{
    return QVariant::fromValue(AbstractJobHandler::OperatorCallback(takeOperatorCallback));
}

std::once_flag g_wiringOnce;

// Subscribe the full production event wiring exactly once for the process:
// real initEventHandle() (dispatcher subscribe + slot channel connect) and
// followEvents() (hook follows). Slot/hook topics must be registered first
// so EventConverter::convert() yields a valid id.
void ensureEventWiring()
{
    std::call_once(g_wiringOnce, []() {
        dfmtest_hooks::registerAllHookEvents();
        dpf::Event::instance()->registerEventType(dpf::EventStratege::kSlot,
                                                  "dfmplugin_fileoperations",
                                                  "slot_Operation_FilesPreview");
        FileOperations ops;
        ops.initialize();
        // hook_Url_IsSubFile is only followed in production after the search
        // plugin starts; follow it directly here so its EventHelper<...> runs.
        dpfHookSequence->follow("dfmplugin_search", "hook_Url_IsSubFile",
                                FileOperationsEventReceiver::instance(),
                                &FileOperationsEventReceiver::handleIsSubFile);
    });
}

}   // namespace

class FileOpsEventDispatchTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetCaptures();

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        ensureEventWiring();
        installCommonStubs();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl createTestFile(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write("dispatch");
        f.close();
        return QUrl::fromLocalFile(path);
    }

    QUrl makeUrl(const QString &name) const
    {
        return QUrl::fromLocalFile(tempDir->path() + "/" + name);
    }

    void installCommonStubs()
    {
        stub.set_lamda(&FileCopyMoveJob::copy,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources, const QUrl &target,
                          const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                           g_copyCalled = true;
                           g_lastCopySources = sources;
                           g_lastCopyTarget = target;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::cut,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources, const QUrl &target,
                          const AbstractJobHandler::JobFlags &, bool) -> JobHandlePointer {
                           g_cutCalled = true;
                           g_lastCutSources = sources;
                           g_lastCutTarget = target;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::deletes,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources,
                          const AbstractJobHandler::JobFlags &, bool) -> JobHandlePointer {
                           g_deletesCalled = true;
                           g_lastDeleteSources = sources;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::moveToTrash,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources,
                          const AbstractJobHandler::JobFlags &flags, bool) -> JobHandlePointer {
                           g_moveToTrashCalled = true;
                           g_lastMoveTrashSources = sources;
                           g_lastMoveTrashFlags = flags;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::restoreFromTrash,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources, const QUrl &target,
                          const AbstractJobHandler::JobFlags &, bool) -> JobHandlePointer {
                           g_restoreFromTrashCalled = true;
                           g_lastRestoreSources = sources;
                           g_lastRestoreTarget = target;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::copyFromTrash,
                       [](FileCopyMoveJob *, const QList<QUrl> &sources, const QUrl &target,
                          const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                           g_copyFromTrashCalled = true;
                           g_lastCopyTrashSources = sources;
                           g_lastCopyTrashTarget = target;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::cleanTrash,
                       [](FileCopyMoveJob *, const QList<QUrl> &urls) -> JobHandlePointer {
                           g_cleanTrashCalled = true;
                           g_lastCleanTrashUrls = urls;
                           return JobHandlePointer(new AbstractJobHandler());
                       });
        stub.set_lamda(&FileCopyMoveJob::initArguments,
                       [](FileCopyMoveJob *, const JobHandlePointer, const AbstractJobHandler::JobFlags &) {
                           g_initArgsCalled = true;
                       });

        stub.set_lamda(&DialogManager::showDeleteFilesDialog, []() -> int {
            g_deleteDialogShown = true;
            return QDialog::Accepted;
        });
        stub.set_lamda(&DialogManager::showClearTrashDialog, []() -> int {
            return QDialog::Accepted;
        });
        stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []() -> int {
            return QDialog::Accepted;
        });
        stub.set_lamda(&DialogManager::showRestoreDeleteFilesDialog, []() -> int {
            return QDialog::Accepted;
        });
        stub.set_lamda(&DialogManager::showDeleteSystemPathWarnDialog, []() {});
        stub.set_lamda(&DialogManager::showNoPermissionDialog, []() {
            g_noPermDialogCalled = true;
        });
        stub.set_lamda(&DialogManager::showErrorDialog, []() {});
        stub.set_lamda(&DialogManager::showRenameNameSameErrorDialog, []() -> int {
            return QDialog::Accepted;
        });
        stub.set_lamda(&DialogManager::showRenameBusyErrDialog, []() {});

        stub.set_lamda(&SystemPathUtil::checkContainsSystemPath,
                       [](SystemPathUtil *, const QList<QUrl> &) -> bool { return false; });
        stub.set_lamda(&FileUtils::fileCanTrash, [](const QUrl &) -> bool { return true; });
        stub.set_lamda(&Application::genericAttribute, [](Application::GenericAttribute ga) -> QVariant {
            return ga == Application::kShowDeleteConfirmDialog ? QVariant(false) : QVariant();
        });

        stub.set_lamda(&LocalFileHandler::mkdir, [](LocalFileHandler *, const QUrl &dir) -> bool {
            g_mkdirCalled = true;
            g_lastMkdirUrl = dir;
            return true;
        });
        stub.set_lamda(&LocalFileHandler::touchFile, [](LocalFileHandler *, const QUrl &url, const QUrl &temp) -> QUrl {
            g_touchFileCalled = true;
            g_lastTouchUrl = url;
            g_lastTouchTempUrl = temp;
            return url;
        });
        stub.set_lamda(&LocalFileHandler::renameFile,
                       [](LocalFileHandler *, const QUrl &oldUrl, const QUrl &newUrl, bool) -> bool {
                           g_renameFileCalled = true;
                           g_lastRenameOld = oldUrl;
                           g_lastRenameNew = newUrl;
                           return true;
                       });
        stub.set_lamda(&LocalFileHandler::renameFilesBatch,
                       [](LocalFileHandler *, const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls) -> bool {
                           g_renameBatchCalled = true;
                           g_lastBatchMap = urls;
                           successUrls = urls;
                           return true;
                       });
        stub.set_lamda(&LocalFileHandler::openFiles, [](LocalFileHandler *, const QList<QUrl> &files) -> bool {
            g_openFilesCalled = true;
            g_lastOpenUrls = files;
            return true;
        });
        stub.set_lamda(&LocalFileHandler::openFilesByApp,
                       [](LocalFileHandler *, const QList<QUrl> &files, const QString &app) -> bool {
                           g_openFilesByAppCalled = true;
                           g_lastOpenUrls = files;
                           g_lastOpenApp = app;
                           return true;
                       });
        stub.set_lamda(&LocalFileHandler::setPermissions,
                       [](LocalFileHandler *, const QUrl &, QFileDevice::Permissions permissions) -> bool {
                           g_setPermsCalled = true;
                           g_lastPerms = permissions;
                           return true;
                       });
        stub.set_lamda(&LocalFileHandler::createSystemLink,
                       [](LocalFileHandler *, const QUrl &source, const QUrl &link) -> bool {
                           g_linkCalled = true;
                           g_lastLinkSource = source;
                           g_lastLinkTarget = link;
                           return true;
                       });

        stub.set_lamda(&ClipBoard::setUrlsToClipboard,
                       [](const QList<QUrl> &list, ClipBoard::ClipboardAction action, QMimeData *) {
                           g_clipboardUrlsCalled = true;
                           g_lastClipUrls = list;
                           g_lastClipAction = action;
                       });
        stub.set_lamda(&ClipBoard::setDataToClipboard, [](QMimeData *mimeData) {
            g_clipboardDataCalled = true;
            g_lastMimeData = mimeData;
        });
        stub.set_lamda(&ClipBoard::replaceClipboardUrl, []() {
            g_replaceClipCalled = true;
        });

        stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                       [](const QString &program, const QStringList &args, const QString &, qint64 *) -> bool {
                           g_detached2Called = true;
                           g_lastDetached2Args = args;
                           g_lastDetachedProgram = program;
                           return true;
                       });

        stub.set_lamda(static_cast<bool (*)(const DDesktopServices::SystemSoundEffect &)>(&DDesktopServices::playSystemSoundEffect),
                       [](const DDesktopServices::SystemSoundEffect &) -> bool {
                           g_soundPlayed = true;
                           return true;
                       });
        stub.set_lamda(&FileUtils::notifyFileChangeManual,
                       [](DFMBASE_NAMESPACE::Global::FileNotifyType, const QUrl &) {});
        stub.set_lamda(&HideFileHelper::save, []() -> bool {
            g_hideSaveCalled = true;
            return true;
        });

        stub.set_lamda(&OperationsStackProxy::saveOperations, []() {
            g_saveOpsCalled = true;
        });
        stub.set_lamda(&OperationsStackProxy::cleanOperations, []() {
            g_cleanOpsCalled = true;
        });
        stub.set_lamda(&OperationsStackProxy::SaveRedoOperations, []() {
            g_saveRedoOpsCalled = true;
        });
        stub.set_lamda(&OperationsStackProxy::CleanOperationsByUrl, []() {
            g_cleanOpsByUrlsCalled = true;
        });
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ========== kCopy / kCutFile / kDeleteFiles ==========

TEST_F(FileOpsEventDispatchTest, CopyEvent_PublishFiveArgs_JobReceivesExactSources)
{
    QUrl src = createTestFile("copy_src.txt");
    QUrl dst = makeUrl("copy_dst_dir");
    QList<QUrl> sources { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCopy, quint64(7), sources, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint,
                                                  makeHandleCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_copyCalled);
    EXPECT_EQ(g_lastCopySources, sources);
    EXPECT_EQ(g_lastCopyTarget, dst);
    EXPECT_TRUE(g_handleCbCalled);
}

TEST_F(FileOpsEventDispatchTest, CopyEvent_PublishSevenArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("copy_cb.txt");
    QUrl dst = makeUrl("copy_cb_dir");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCopy, quint64(9), QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("copy-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_copyCalled);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("copy-custom"));
    EXPECT_EQ(g_opCb.windowId, quint64(9));
    EXPECT_TRUE(g_opCb.hasHandle);
}

TEST_F(FileOpsEventDispatchTest, CutEvent_PublishFiveArgs_JobReceivesExactSources)
{
    QUrl src = createTestFile("cut_src.txt");
    QUrl dst = makeUrl("cut_dst_dir");
    QList<QUrl> sources { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCutFile, quint64(3), sources, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_cutCalled);
    EXPECT_EQ(g_lastCutSources, sources);
    EXPECT_EQ(g_lastCutTarget, dst);
}

TEST_F(FileOpsEventDispatchTest, CutEvent_PublishSevenArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("cut_cb.txt");
    QUrl dst = makeUrl("cut_cb_dir");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCutFile, quint64(4), QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("cut-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_cutCalled);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("cut-custom"));
    EXPECT_EQ(g_opCb.windowId, quint64(4));
}

TEST_F(FileOpsEventDispatchTest, DeleteEvent_PublishFourArgs_DeleteJobRunsAfterConfirm)
{
    QUrl src = createTestFile("del_src.txt");
    QList<QUrl> sources { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, quint64(5), sources,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_deleteDialogShown);
    ASSERT_TRUE(g_deletesCalled);
    EXPECT_EQ(g_lastDeleteSources, sources);
}

TEST_F(FileOpsEventDispatchTest, DeleteEvent_PublishSixArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("del_cb.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, quint64(6), QList<QUrl> { src },
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("del-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_deletesCalled);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("del-custom"));
    EXPECT_EQ(g_opCb.windowId, quint64(6));
}

// ========== kOpenFiles / kOpenFilesByApp / kOpenInTerminal ==========

TEST_F(FileOpsEventDispatchTest, OpenFilesEvent_PublishTwoArgs_FilesOpened)
{
    QUrl src = createTestFile("open_me.txt");
    QList<QUrl> urls { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, quint64(2), urls);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_openFilesCalled);
    EXPECT_EQ(g_lastOpenUrls, urls);
    EXPECT_FALSE(g_clipboardDataCalled);
}

TEST_F(FileOpsEventDispatchTest, OpenFilesEvent_PublishThreeArgs_BoolOutParamPreserved)
{
    QUrl src = createTestFile("open_boolp.txt");
    bool okFlag = false;

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, quint64(2),
                                                  QList<QUrl> { src }, QVariant::fromValue(&okFlag));

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_openFilesCalled);
    // local file: hook does not intercept, out-param must stay untouched
    EXPECT_FALSE(okFlag);
}

TEST_F(FileOpsEventDispatchTest, OpenFilesEvent_PublishFourArgs_CallbackReportsSuccess)
{
    QUrl src = createTestFile("open_cb.txt");
    QList<QUrl> urls { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, quint64(8), urls,
                                                  QVariant("open-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("open-custom"));
    EXPECT_TRUE(g_opCb.success);
    EXPECT_EQ(g_opCb.sourceUrls, urls);
}

TEST_F(FileOpsEventDispatchTest, OpenFilesByAppEvent_PublishThreeArgs_AppNamePassedThrough)
{
    QUrl src = createTestFile("byapp.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, quint64(11),
                                                  QList<QUrl> { src }, QList<QString> { "org.test.editor" });

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_openFilesByAppCalled);
    EXPECT_EQ(g_lastOpenApp, QString("org.test.editor"));
}

TEST_F(FileOpsEventDispatchTest, OpenFilesByAppEvent_PublishFiveArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("byapp_cb.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, quint64(12),
                                                  QList<QUrl> { src }, QList<QString> { "org.test.editor" },
                                                  QVariant("app-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("app-custom"));
    EXPECT_TRUE(g_openFilesByAppCalled);
}

TEST_F(FileOpsEventDispatchTest, OpenInTerminalEvent_PublishTwoArgs_ProcessDetached)
{
    QDir().mkpath(tempDir->path() + "/term_dir");
    QUrl src = createTestFile("term_dir/terminal_here.txt");

    // force a terminal to be found regardless of the host environment
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFileInfo::exists),
                   [](const QString &) -> bool { return true; });

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenInTerminal, quint64(13), QList<QUrl> { src });

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_detached2Called);
    EXPECT_FALSE(g_lastDetachedProgram.isEmpty());
    EXPECT_TRUE(g_lastDetached2Args.isEmpty() || g_lastDetached2Args.contains("-w"));
}

// ========== kRenameFile / kRenameFiles ==========

TEST_F(FileOpsEventDispatchTest, RenameFileEvent_PublishFourArgs_RenameDoneAndOperationSaved)
{
    stub.set_lamda(&OperationsStackProxy::saveOperations,
                   [](OperationsStackProxy *, const QVariantMap &values) {
                       g_saveOpsCalled = true;
                       g_lastSavedOps = values;
                   });

    QUrl oldUrl = createTestFile("rn_old.txt");
    QUrl newUrl = makeUrl("rn_new.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, quint64(14), oldUrl, newUrl,
                                                  AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_renameFileCalled);
    EXPECT_EQ(g_lastRenameOld, oldUrl);
    EXPECT_EQ(g_lastRenameNew, newUrl);
    ASSERT_TRUE(g_saveOpsCalled);
    EXPECT_EQ(g_lastSavedOps.value("undoevent").value<uint16_t>(),
              static_cast<uint16_t>(GlobalEventType::kRenameFile));
    EXPECT_EQ(g_lastSavedOps.value("undosources").toStringList(), QUrl::toStringList({ newUrl }));
}

TEST_F(FileOpsEventDispatchTest, RenameFileEvent_PublishSixArgs_OperatorCallbackGetsCustom)
{
    QUrl oldUrl = createTestFile("rn_cb_old.txt");
    QUrl newUrl = makeUrl("rn_cb_new.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, quint64(15), oldUrl, newUrl,
                                                  AbstractJobHandler::JobFlag::kNoHint, QVariant("rn-custom"),
                                                  makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("rn-custom"));
    EXPECT_TRUE(g_renameFileCalled);
}

TEST_F(FileOpsEventDispatchTest, RenameFilesEvent_ReplaceText_BatchRenameMapGenerated)
{
    QUrl src = createTestFile("aa_1.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, quint64(16), QList<QUrl> { src },
                                                  qMakePair(QString("aa"), QString("bb")), true);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_renameBatchCalled);
    ASSERT_EQ(g_lastBatchMap.size(), 1);
    EXPECT_EQ(g_lastBatchMap.firstKey(), src);
    EXPECT_EQ(g_lastBatchMap.first().fileName(), QString("bb_1.txt"));
}

TEST_F(FileOpsEventDispatchTest, RenameFilesEvent_ReplaceWithCallback_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("cbrep.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, quint64(17), QList<QUrl> { src },
                                                  qMakePair(QString("cbrep"), QString("xcbrep")), false,
                                                  QVariant("rep-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("rep-custom"));
    EXPECT_TRUE(g_renameBatchCalled);
}

TEST_F(FileOpsEventDispatchTest, RenameFilesEvent_AddPrefixText_UrlsPrefixed)
{
    QUrl src = createTestFile("addme.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, quint64(18), QList<QUrl> { src },
                                                  qMakePair(QString("pre_"),
                                                            AbstractJobHandler::FileNameAddFlag::kPrefix));

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_renameBatchCalled);
    ASSERT_EQ(g_lastBatchMap.size(), 1);
    EXPECT_EQ(g_lastBatchMap.firstKey(), src);
    EXPECT_EQ(g_lastBatchMap.first().fileName(), QString("pre_addme.txt"));
}

TEST_F(FileOpsEventDispatchTest, RenameFilesEvent_AddTextWithCallback_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("appended.log");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, quint64(19), QList<QUrl> { src },
                                                  qMakePair(QString("_tail"),
                                                            AbstractJobHandler::FileNameAddFlag::kSuffix),
                                                  QVariant("add-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("add-custom"));
    EXPECT_TRUE(g_renameBatchCalled);
}

// ========== kMkdir / kTouchFile ==========

TEST_F(FileOpsEventDispatchTest, MkdirEvent_PublishTwoArgs_DirectoryCreatedInTargetDir)
{
    stub.set_lamda(&OperationsStackProxy::saveOperations,
                   [](OperationsStackProxy *, const QVariantMap &values) {
                       g_saveOpsCalled = true;
                       g_lastSavedOps = values;
                   });

    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kMkdir, quint64(20), dirUrl);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_mkdirCalled);
    EXPECT_EQ(g_lastMkdirUrl.scheme(), QString("file"));
    EXPECT_TRUE(g_lastMkdirUrl.path().startsWith(tempDir->path() + "/"));
    ASSERT_TRUE(g_saveOpsCalled);
    EXPECT_EQ(g_lastSavedOps.value("redoevent").value<uint16_t>(),
              static_cast<uint16_t>(GlobalEventType::kMkdir));
}

TEST_F(FileOpsEventDispatchTest, MkdirEvent_PublishFourArgs_OperatorCallbackGetsCustom)
{
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kMkdir, quint64(21), dirUrl,
                                                  QVariant("mkdir-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("mkdir-custom"));
    EXPECT_TRUE(g_mkdirCalled);
}

TEST_F(FileOpsEventDispatchTest, TouchFileEvent_CreateTextFile_SuffixAppliedAndSaved)
{
    stub.set_lamda(&OperationsStackProxy::saveOperations,
                   [](OperationsStackProxy *, const QVariantMap &values) {
                       g_saveOpsCalled = true;
                       g_lastSavedOps = values;
                   });

    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, quint64(22), dirUrl,
                                                  Global::CreateFileType::kCreateFileTypeText, QString("txt"));

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_touchFileCalled);
    EXPECT_TRUE(g_lastTouchUrl.path().endsWith(".txt"));
    EXPECT_TRUE(g_lastTouchUrl.path().startsWith(tempDir->path() + "/"));
    ASSERT_TRUE(g_saveOpsCalled);
    EXPECT_EQ(g_lastSavedOps.value("redoevent").value<uint16_t>(),
              static_cast<uint16_t>(GlobalEventType::kTouchFile));
}

TEST_F(FileOpsEventDispatchTest, TouchFileEvent_FromTemplate_TemplateUrlForwarded)
{
    QUrl tpl = createTestFile("tpl.odt");
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, quint64(23), dirUrl, tpl,
                                                  QString());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_touchFileCalled);
    EXPECT_TRUE(g_lastTouchUrl.path().endsWith(".odt"));
    EXPECT_TRUE(g_lastTouchUrl.path().startsWith(tempDir->path() + "/"));
    // the template overload must have forwarded the real template url
    EXPECT_EQ(g_lastTouchTempUrl, tpl);
}

TEST_F(FileOpsEventDispatchTest, TouchFileEvent_CreateWithCallback_OperatorCallbackGetsCustom)
{
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, quint64(24), dirUrl,
                                                  Global::CreateFileType::kCreateFileTypeText, QString("txt"),
                                                  QVariant("touch-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("touch-custom"));
    EXPECT_TRUE(g_touchFileCalled);
}

TEST_F(FileOpsEventDispatchTest, TouchFileEvent_TemplateWithCallback_OperatorCallbackGetsCustom)
{
    QUrl tpl = createTestFile("tpl2.xlsx");
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, quint64(25), dirUrl, tpl,
                                                  QString("xlsx"), QVariant("tpl-custom"),
                                                  makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("tpl-custom"));
    EXPECT_EQ(g_lastTouchTempUrl, tpl);
}

// ========== kCreateSymlink / kSetPermission ==========

TEST_F(FileOpsEventDispatchTest, CreateSymlinkEvent_SilentExplicitTarget_LinkCreated)
{
    QUrl src = createTestFile("link_src.txt");
    QUrl link = makeUrl("the_link.lnk");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, quint64(26), src, link,
                                                  false, true);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_linkCalled);
    EXPECT_EQ(g_lastLinkSource, src);
    EXPECT_EQ(g_lastLinkTarget, link);
}

TEST_F(FileOpsEventDispatchTest, CreateSymlinkEvent_WithCallback_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("link_cb.txt");
    QUrl link = makeUrl("cb_link.lnk");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, quint64(27), src, link,
                                                  false, true, QVariant("link-custom"),
                                                  makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("link-custom"));
    EXPECT_TRUE(g_linkCalled);
}

TEST_F(FileOpsEventDispatchTest, SetPermissionEvent_PublishThreeArgs_ExactPermissionsApplied)
{
    QUrl src = createTestFile("perm.txt");
    QFileDevice::Permissions perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner;

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kSetPermission, quint64(28), src, perms);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_setPermsCalled);
    EXPECT_EQ(g_lastPerms, perms);
    EXPECT_EQ(int(g_lastPerms), 0x6000);
}

TEST_F(FileOpsEventDispatchTest, SetPermissionEvent_PublishFiveArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("perm_cb.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kSetPermission, quint64(29), src,
                                                  QFileDevice::ReadOwner, QVariant("perm-custom"),
                                                  makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("perm-custom"));
    EXPECT_TRUE(g_setPermsCalled);
}

// ========== clipboard events ==========

TEST_F(FileOpsEventDispatchTest, WriteUrlsToClipboardEvent_CutAction_ActionAndUrlsDelivered)
{
    QUrl src = createTestFile("clip.txt");
    QList<QUrl> urls { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, quint64(30),
                                                  ClipBoard::ClipboardAction::kCutAction, urls);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_clipboardUrlsCalled);
    EXPECT_EQ(g_lastClipAction, ClipBoard::ClipboardAction::kCutAction);
    EXPECT_EQ(g_lastClipUrls, urls);
}

TEST_F(FileOpsEventDispatchTest, WriteCustomClipboardEvent_ValidMimeData_DataForwarded)
{
    QMimeData data;
    data.setText("mime-payload");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kWriteCustomToClipboard, quint64(31), &data);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_clipboardDataCalled);
    EXPECT_EQ(g_lastMimeData, &data);
}

TEST_F(FileOpsEventDispatchTest, WriteCustomClipboardEvent_NullMimeData_Rejected)
{
    bool published = dpfSignalDispatcher->publish(GlobalEventType::kWriteCustomToClipboard, quint64(32),
                                                  static_cast<QMimeData *>(nullptr));

    EXPECT_TRUE(published);
    EXPECT_FALSE(g_clipboardDataCalled);
}

// ========== operation-stack events ==========

TEST_F(FileOpsEventDispatchTest, SaveOperatorEvent_OperationMap_SavedVerbatim)
{
    QVariantMap values;
    values.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kCopy)));
    values.insert("undosources", QUrl::toStringList({ makeUrl("s1.txt") }));

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, values);

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_saveOpsCalled);
    EXPECT_FALSE(g_saveRedoOpsCalled);
}

TEST_F(FileOpsEventDispatchTest, SaveRedoOperatorEvent_OperationMap_SavedVerbatim)
{
    QVariantMap values;
    values.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kRenameFile)));

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kSaveRedoOperator, values);

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_saveRedoOpsCalled);
    EXPECT_FALSE(g_saveOpsCalled);
}

TEST_F(FileOpsEventDispatchTest, CleanSaveOperatorEvent_NoParams_OperationsStackCleaned)
{
    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCleanSaveOperator);

    EXPECT_TRUE(published);
    EXPECT_TRUE(g_cleanOpsCalled);
    EXPECT_FALSE(g_cleanOpsByUrlsCalled);
}

TEST_F(FileOpsEventDispatchTest, CleanSaveOperatorByUrlsEvent_UrlList_ConvertedToStrings)
{
    stub.set_lamda(&OperationsStackProxy::CleanOperationsByUrl,
                   [](OperationsStackProxy *, const QStringList &urls) {
                       g_cleanOpsByUrlsCalled = true;
                       g_lastCleanOpsUrls = urls;
                   });

    QList<QUrl> urls { makeUrl("gone1.txt"), makeUrl("gone2.txt") };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCleanSaveOperatorByUrls, urls);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_cleanOpsByUrlsCalled);
    EXPECT_EQ(g_lastCleanOpsUrls, QUrl::toStringList(urls));
    EXPECT_EQ(g_lastCleanOpsUrls.size(), 2);
}

TEST_F(FileOpsEventDispatchTest, CopyFilePathEvent_LocalUrls_PathsWrittenToClipboard)
{
    stub.set_lamda(&ClipBoard::setDataToClipboard, [](QMimeData *mimeData) {
        g_clipboardDataCalled = true;
        g_lastClipboardText = mimeData->text();
    });

    QUrl src = createTestFile("pathcopy.txt");
    QList<QUrl> urls { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCopyFilePath, urls);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_clipboardDataCalled);
    EXPECT_EQ(g_lastClipboardText, src.toLocalFile());
    EXPECT_FALSE(g_lastClipboardText.isEmpty());
}

// ========== undo / redo events ==========

TEST_F(FileOpsEventDispatchTest, RevocationEvent_UndoCut_CutJobAndInitArgumentsInvoked)
{
    QUrl src = createTestFile("undo_cut.txt");
    QUrl dst = makeUrl("undo_cut_dst");

    QVariantMap op;
    op.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kCutFile)));
    op.insert("undosources", QUrl::toStringList({ src }));
    op.insert("undotargets", QUrl::toStringList({ dst }));
    stub.set_lamda(&OperationsStackProxy::revocationOperations,
                   [&op](OperationsStackProxy *) -> QVariantMap { return op; });

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRevocation, quint64(33),
                                                  makeHandleCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_cutCalled);
    EXPECT_EQ(g_lastCutSources, QList<QUrl> { src });
    EXPECT_EQ(g_lastCutTarget, dst);
    EXPECT_TRUE(g_initArgsCalled);
    EXPECT_TRUE(g_handleCbCalled);
}

TEST_F(FileOpsEventDispatchTest, RedoEvent_UndoMkdir_MkdirReexecutedWithUrlPath)
{
    QUrl dirUrl = makeUrl("redo_dir");

    QVariantMap op;
    op.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kMkdir)));
    op.insert("undosources", QUrl::toStringList({ dirUrl }));
    op.insert("undotargets", QUrl::toStringList({ dirUrl }));
    stub.set_lamda(&OperationsStackProxy::RevocationRedoOperations,
                   [&op](OperationsStackProxy *) -> QVariantMap { return op; });

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRedo, quint64(34), nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_mkdirCalled);
    EXPECT_EQ(g_lastMkdirUrl, dirUrl);
}

// ========== kHideFiles ==========

TEST_F(FileOpsEventDispatchTest, HideFilesEvent_PublishTwoArgs_HiddenListSaved)
{
    QUrl src = createTestFile("hide_me.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kHideFiles, quint64(35), QList<QUrl> { src });

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_hideSaveCalled);
    EXPECT_FALSE(g_noPermDialogCalled);
}

TEST_F(FileOpsEventDispatchTest, HideFilesEvent_ParentWithList_FileMarkedHidden)
{
    QUrl parent = QUrl::fromLocalFile(tempDir->path());
    QUrl src = createTestFile("hide_flagged.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kHideFiles, quint64(36), parent,
                                                  QList<QUrl> { src }, true);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_hideSaveCalled);
    EXPECT_FALSE(g_cleanOpsCalled);
}

TEST_F(FileOpsEventDispatchTest, HideFilesEvent_WithCallback_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("hide_cb.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kHideFiles, quint64(37), QList<QUrl> { src },
                                                  QVariant("hide-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("hide-custom"));
    EXPECT_TRUE(g_hideSaveCalled);
}

// ========== slot channel + hook strategies ==========

TEST_F(FileOpsEventDispatchTest, FilesPreviewSlot_PushValidLists_PreviewToolLaunchedWithTempFile)
{
    QUrl selected = createTestFile("preview.txt");
    QList<QUrl> selectUrls { selected };
    QList<QUrl> dirUrls { QUrl::fromLocalFile(tempDir->path()) };

    QVariant ret = dpfSlotChannel->push("dfmplugin_fileoperations", "slot_Operation_FilesPreview",
                                        quint64(38), selectUrls, dirUrls);

    EXPECT_TRUE(ret.isNull());
    ASSERT_TRUE(g_detached2Called);
    ASSERT_EQ(g_lastDetached2Args.size(), 1);
    QString tempFileName = g_lastDetached2Args.first();
    EXPECT_TRUE(QFileInfo(tempFileName).exists());
    QFile::remove(tempFileName);   // handler sets autoRemove(false): clean up
}

TEST_F(FileOpsEventDispatchTest, IsSubFileHook_RunNestedUrl_ReturnsTrue)
{
    QUrl parent = QUrl::fromLocalFile(tempDir->path() + "/nested");
    QUrl sub = QUrl::fromLocalFile(tempDir->path() + "/nested/inner/file.txt");

    bool nested = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile", parent, sub);
    bool foreign = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile",
                                        QUrl::fromLocalFile(tempDir->path() + "/other"), sub);

    EXPECT_TRUE(nested);
    EXPECT_FALSE(foreign);
}

TEST_F(FileOpsEventDispatchTest, ShortcutHooks_WritableTargets_NotBlocked)
{
    QUrl root = QUrl::fromLocalFile(tempDir->path());
    QUrl src = createTestFile("shortcut.txt");
    QList<QUrl> urls { src };

    bool deleteBlocked = dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_DeleteFiles",
                                              quint64(39), urls, root);
    bool pasteBlocked = dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                             quint64(40), urls, root);

    EXPECT_FALSE(deleteBlocked);
    EXPECT_FALSE(pasteBlocked);
    EXPECT_FALSE(g_noPermDialogCalled);
}

TEST_F(FileOpsEventDispatchTest, RevocationEvent_UndoMoveToTrash_TrashJobAndInitArgumentsInvoked)
{
    QUrl src = createTestFile("undo_trash.txt");

    QVariantMap op;
    op.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kMoveToTrash)));
    op.insert("undosources", QUrl::toStringList({ src }));
    op.insert("undotargets", QUrl::toStringList({ makeUrl("nowhere") }));
    stub.set_lamda(&OperationsStackProxy::revocationOperations,
                   [&op](OperationsStackProxy *) -> QVariantMap { return op; });

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRevocation, quint64(49), nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_moveToTrashCalled);
    EXPECT_EQ(g_lastMoveTrashSources, QList<QUrl> { src });
    EXPECT_EQ(g_lastMoveTrashFlags,
              AbstractJobHandler::JobFlags(AbstractJobHandler::JobFlag::kRevocation));
    EXPECT_TRUE(g_initArgsCalled);
}

TEST_F(FileOpsEventDispatchTest, RevocationEvent_UndoRestoreFromTrash_RestoreJobInvoked)
{
    QUrl src = makeUrl("trash/undo_restore.txt");

    QVariantMap op;
    op.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(GlobalEventType::kRestoreFromTrash)));
    op.insert("undosources", QUrl::toStringList({ src }));
    op.insert("undotargets", QUrl::toStringList({ src }));
    stub.set_lamda(&OperationsStackProxy::revocationOperations,
                   [&op](OperationsStackProxy *) -> QVariantMap { return op; });

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRevocation, quint64(50),
                                                  makeHandleCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_restoreFromTrashCalled);
    EXPECT_EQ(g_lastRestoreSources, QList<QUrl> { src });
    EXPECT_TRUE(g_initArgsCalled);
    EXPECT_TRUE(g_handleCbCalled);
}

TEST_F(FileOpsEventDispatchTest, TouchFileEvent_ClipboardImageRequest_PngSavedFromClipboard)
{
    QImage image(4, 4, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QApplication::clipboard()->setImage(image);

    QVariantMap custom;
    custom.insert("clipboardImage", true);
    QUrl dirUrl = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, quint64(51), dirUrl,
                                                  Global::CreateFileType::kCreateFileTypeText,
                                                  QString("png"), custom, nullptr);

    EXPECT_TRUE(published);
    QDir dir(tempDir->path());
    // other fixtures may have added duplicate subscriptions of the same
    // handler, so at least one image must exist, each with a proper name
    const auto pngs = dir.entryList({ "image_*.png" }, QDir::Files);
    EXPECT_GE(pngs.size(), 1);
    for (const QString &name : pngs) {
        EXPECT_TRUE(name.startsWith("image_"));
        EXPECT_GT(QFile(dir.filePath(name)).size(), 0);
    }
}

// ========== TrashFileEventReceiver events ==========

TEST_F(FileOpsEventDispatchTest, MoveToTrashEvent_PublishFourArgs_TrashJobStarted)
{
    QUrl src = createTestFile("totrash.txt");
    QList<QUrl> sources { src };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, quint64(41), sources,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_moveToTrashCalled);
    EXPECT_EQ(g_lastMoveTrashSources, sources);
    EXPECT_EQ(g_lastMoveTrashFlags, AbstractJobHandler::JobFlags(AbstractJobHandler::JobFlag::kNoHint));
}

TEST_F(FileOpsEventDispatchTest, MoveToTrashEvent_PublishSixArgs_OperatorCallbackGetsCustom)
{
    QUrl src = createTestFile("totrash_cb.txt");

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, quint64(42), QList<QUrl> { src },
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("trash-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("trash-custom"));
    EXPECT_TRUE(g_moveToTrashCalled);
}

TEST_F(FileOpsEventDispatchTest, RestoreFromTrashEvent_PublishFiveArgs_RestoreJobStarted)
{
    QUrl src = makeUrl("trash/item.txt");
    QUrl dst = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash, quint64(43),
                                                  QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_restoreFromTrashCalled);
    EXPECT_EQ(g_lastRestoreSources, QList<QUrl> { src });
    EXPECT_EQ(g_lastRestoreTarget, dst);
}

TEST_F(FileOpsEventDispatchTest, RestoreFromTrashEvent_PublishSevenArgs_OperatorCallbackGetsCustom)
{
    QUrl src = makeUrl("trash/item_cb.txt");
    QUrl dst = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash, quint64(44),
                                                  QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("restore-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("restore-custom"));
    EXPECT_TRUE(g_restoreFromTrashCalled);
}

TEST_F(FileOpsEventDispatchTest, CopyFromTrashEvent_PublishFiveArgs_CopyJobStarted)
{
    QUrl src = makeUrl("trash/copyme.txt");
    QUrl dst = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCopyFromTrash, quint64(45),
                                                  QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_copyFromTrashCalled);
    EXPECT_EQ(g_lastCopyTrashSources, QList<QUrl> { src });
    EXPECT_EQ(g_lastCopyTrashTarget, dst);
}

TEST_F(FileOpsEventDispatchTest, CopyFromTrashEvent_PublishSevenArgs_OperatorCallbackGetsCustom)
{
    QUrl src = makeUrl("trash/copyme_cb.txt");
    QUrl dst = QUrl::fromLocalFile(tempDir->path());

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCopyFromTrash, quint64(46),
                                                  QList<QUrl> { src }, dst,
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr,
                                                  QVariant("copytrash-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("copytrash-custom"));
    EXPECT_TRUE(g_copyFromTrashCalled);
}

TEST_F(FileOpsEventDispatchTest, CleanTrashEvent_PublishFourArgs_TrashCleanedAfterConfirm)
{
    QList<QUrl> urls { makeUrl("trash/stale1.txt"), makeUrl("trash/stale2.txt") };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash, quint64(47), urls,
                                                  AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_deleteDialogShown);
    ASSERT_TRUE(g_cleanTrashCalled);
    EXPECT_EQ(g_lastCleanTrashUrls, urls);
    EXPECT_TRUE(g_soundPlayed);
}

TEST_F(FileOpsEventDispatchTest, CleanTrashEvent_PublishFiveArgs_OperatorCallbackGetsCustom)
{
    QList<QUrl> urls { makeUrl("trash/cb1.txt") };

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash, quint64(48), urls, nullptr,
                                                  QVariant("clean-custom"), makeOperatorCallback());

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_opCb.called);
    EXPECT_EQ(g_opCb.custom.toString(), QString("clean-custom"));
    EXPECT_TRUE(g_cleanTrashCalled);
    EXPECT_TRUE(g_opCb.hasHandle);
}
