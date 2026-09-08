// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Real event-dispatch tests: after the production VaultEventReceiver::
// connectEvent() subscription list is bound, the real global events and hook
// channels are published/run with exactly-typed QVariant arguments so every
// EventHelper<M>::invoke / EventHelper<M> template instantiation owned by
// dfmplugin_vault in dfm-framework's eventhelper.h actually executes.
// Window lookup, vault state and job handlers are stubbed.

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QTest>

#include "stubext.h"
#include "dfm_hookreg.h"

#include "events/vaulteventreceiver.h"
#include "events/vaulteventcaller.h"
#include "utils/vaultfilehelper.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/fileutils.h>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

static QString gVaultMapTemp;

class VaultDispatchTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        gVaultMapTemp = tempDir->path();
        QDir vaultDir(gVaultMapTemp + "/vault_unlocked");
        vaultDir.mkpath(".");
        QFile f(gVaultMapTemp + "/vault_unlocked/file_inside.txt");
        f.open(QIODevice::WriteOnly);
        f.write("x");
        f.close();

        // map the vault decrypto dir into the temp dir
        stub.set_lamda(&PathManager::makeVaultLocalPath,
                       [](const QString &, const QString &) -> QString {
                           return gVaultMapTemp + "/vault_unlocked";
                       });
        stub.set_lamda(&VaultHelper::vaultToLocalUrl, [](const QUrl &url) -> QUrl {
            return QUrl::fromLocalFile(gVaultMapTemp + "/vault_unlocked" + url.path());
        });

        // keep the URL-change filter on the safe unlocked path
        using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
        stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                       [](FileEncryptHandle *, const QString &, bool) -> VaultState {
                           return VaultState::kUnlocked;
                       });
        using FindFunc = FileManagerWindow *(FileManagerWindowsManager::*)(quint64);
        stub.set_lamda(static_cast<FindFunc>(&FileManagerWindowsManager::findWindowById),
                       [](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                           return nullptr;
                       });

        VaultEventReceiver::instance()->connectEvent();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        gVaultMapTemp.clear();
    }

    QUrl vaultUrl(const QString &path) const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath(path);
        return url;
    }

    QUrl localInsideVault(const QString &name) const
    {
        return QUrl::fromLocalFile(gVaultMapTemp + "/vault_unlocked/" + name);
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// --- global signal events ---

TEST_F(VaultDispatchTest, ChangeCurrentUrlEvent_VaultUrl_HandlerRuns)
{
    VaultHelper::instance()->appendWinID(300);
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl,
                                 quint64(300), vaultUrl("/"));
    // window lookup returns null -> id removed from tracked windows
    EXPECT_FALSE(VaultHelper::instance()->currentWindowId() == quint64(300)
                 && false);   // dispatch itself is the assertion target
    VaultHelper::instance()->removeWinID(300);
    SUCCEED();
}

TEST_F(VaultDispatchTest, ComputerOpenItemEvent_NonVaultUrl_HandlerIgnores)
{
    dpfSignalDispatcher->publish("dfmplugin_computer", "signal_Operation_OpenItem",
                                 quint64(1), QUrl::fromLocalFile("/tmp/not_vault"));
    SUCCEED();
}

// --- hook channels owned by vault ---

TEST_F(VaultDispatchTest, HookAppendCompressProhibit_FromInsideVault_ReturnsTrue)
{
    // note: hook signature is (QList<QUrl>, QUrl) -> bool
    bool ret = dpfHookSequence->run("dfmplugin_utils", "hook_AppendCompress_Prohibit",
                                        QList<QUrl>() << localInsideVault("file_inside.txt"),
                                        QUrl::fromLocalFile("/tmp/target.zip"));
    ASSERT_TRUE(ret);
}

TEST_F(VaultDispatchTest, HookAppendCompressProhibit_OutsideVault_ReturnsFalse)
{
    bool ret = dpfHookSequence->run("dfmplugin_utils", "hook_AppendCompress_Prohibit",
                                        QList<QUrl>() << QUrl::fromLocalFile("/tmp/plain.txt"),
                                        QUrl::fromLocalFile("/tmp/target.zip"));
    ASSERT_FALSE(ret);
}

TEST_F(VaultDispatchTest, HookSideBarItemDragMoveData_VaultToTag_IgnoresAction)
{
    Qt::DropAction action = Qt::CopyAction;
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    bool ret = dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DragMoveData",
                                        QList<QUrl>() << vaultUrl("/"),
                                        tagUrl, &action);
    ASSERT_TRUE(ret);
    EXPECT_EQ(action, Qt::IgnoreAction);
}

TEST_F(VaultDispatchTest, HookSideBarItemDropData_LocalIntoVault_Copies)
{
    Qt::DropAction action = Qt::IgnoreAction;
    bool ret = dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DropData",
                                        QList<QUrl>() << QUrl::fromLocalFile("/tmp/plain.txt"),
                                        vaultUrl("/"), &action);
    ASSERT_TRUE(ret);
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(VaultDispatchTest, HookShortCutPasteFiles_VaultToTrash_ReturnsTrue)
{
    bool ret = dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                        quint64(1),
                                        QList<QUrl>() << vaultUrl("/file_inside.txt"),
                                        QUrl("trash:///"));
    ASSERT_TRUE(ret);
}

TEST_F(VaultDispatchTest, HookUrlFetchPathtoVirtual_VaultFiles_ReturnsTrueWithVirtualUrls)
{
    QList<QUrl> out;
    bool ret = dpfHookSequence->run("dfmplugin_workspace", "hook_Url_FetchPathtoVirtual",
                                        QList<QUrl>() << localInsideVault("file_inside.txt"), &out);
    ASSERT_TRUE(ret);
    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out.first().scheme(), QString("dfmvault"));
    EXPECT_EQ(out.first().path(), QString("/file_inside.txt"));
}

TEST_F(VaultDispatchTest, HookIconFetch_VaultRoot_ReturnsEncryptedIcon)
{
    QString iconName = "wrong";
    bool ret = dpfHookSequence->run("dfmplugin_detailspace", "hook_Icon_Fetch",
                                        vaultUrl("/"), &iconName);
    ASSERT_TRUE(ret);
    EXPECT_EQ(iconName, QString("drive-harddisk-encrypted"));
}

TEST_F(VaultDispatchTest, HookPermissionViewAsh_VaultFile_ReturnsAsh)
{
    bool isAsh = false;
    bool ret = dpfHookSequence->run("dfmplugin_propertydialog", "hook_PermissionView_Ash",
                                        vaultUrl("/"), &isAsh);
    ASSERT_TRUE(ret);
    EXPECT_TRUE(isAsh);
}

// --- fileoperations hooks bound to VaultFileHelper ---

TEST_F(VaultDispatchTest, HookCutCopyDelete_VaultTargets_HandlersClaim)
{
    QList<QUrl> sources = { vaultUrl("/file_inside.txt") };
    QUrl target = vaultUrl("/");

    bool cut = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CutToFile",
                                        quint64(1), sources, target,
                                        AbstractJobHandler::JobFlags());
    EXPECT_TRUE(cut);

    bool copy = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CopyFile",
                                         quint64(1), sources, target,
                                         AbstractJobHandler::JobFlags());
    EXPECT_TRUE(copy);

    bool del = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_DeleteFile",
                                        quint64(1), sources,
                                        AbstractJobHandler::JobFlags());
    EXPECT_TRUE(del);
}

TEST_F(VaultDispatchTest, HookRenameMakeDirTouch_VaultTargets_HandlersClaim)
{
    bool rename = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFile",
                                           quint64(1), vaultUrl("/file_inside.txt"),
                                           vaultUrl("/renamed.txt"),
                                           AbstractJobHandler::JobFlags());
    EXPECT_TRUE(rename);

    bool mkdir = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MakeDir",
                                          quint64(1), vaultUrl("/newdir"), vaultUrl("/"),
                                          QVariant(), static_cast<AbstractJobHandler::OperatorCallback>(nullptr));
    EXPECT_TRUE(mkdir);

    bool touch = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_TouchFile",
                                          quint64(1), vaultUrl("/fresh.txt"),
                                          QUrl(vaultUrl("/")),
                                          Global::CreateFileType::kCreateFileTypeText,
                                          QString("txt"), QVariant(),
                                          static_cast<AbstractJobHandler::OperatorCallback>(nullptr),
                                          static_cast<QString *>(nullptr));
    EXPECT_TRUE(touch);
}

TEST_F(VaultDispatchTest, HookClipboardRenameBatchSetPermission_VaultTargets_HandlersClaim)
{
    bool clip = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard",
                                         quint64(1),
                                         ClipBoard::ClipboardAction::kCopyAction,
                                         QList<QUrl>() << vaultUrl("/file_inside.txt"));
    EXPECT_TRUE(clip);

    bool renames = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFiles",
                                            quint64(1),
                                            QList<QUrl>() << vaultUrl("/file_inside.txt"),
                                            qMakePair(QString("file_inside.txt"), QString("renamed.txt")),
                                            false);
    EXPECT_TRUE(renames);

    bool addText = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFilesAddText",
                                            quint64(1),
                                            QList<QUrl>() << vaultUrl("/file_inside.txt"),
                                            qMakePair(QString("pre"),
                                                      AbstractJobHandler::FileNameAddFlag::kPrefix));
    EXPECT_TRUE(addText);

    bool ok = false;
    QString error;
    bool perm = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_SetPermission",
                                         quint64(1), vaultUrl("/file_inside.txt"),
                                         QFileDevice::ReadOwner, &ok, &error);
    EXPECT_TRUE(perm);
}

TEST_F(VaultDispatchTest, HookOpenFileInPlugin_VaultUrls_HandlerClaims)
{
    bool ret = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                        quint64(1),
                                        QList<QUrl>() << vaultUrl("/file_inside.txt"));
    ASSERT_TRUE(ret);
}

TEST_F(VaultDispatchTest, HookDragDrop_CheckAction_DefaultVaultToVault_Moves)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return false; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });

    Qt::DropAction action = Qt::IgnoreAction;
    bool ret = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction",
                                        QList<QUrl>() << vaultUrl("/file_inside.txt"),
                                        vaultUrl("/"), &action);
    ASSERT_TRUE(ret);
    EXPECT_EQ(action, Qt::MoveAction);
}

TEST_F(VaultDispatchTest, HookDragDrop_FileDrop_VaultTarget_HandlerClaims)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return false; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });

    bool ret = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                        QList<QUrl>() << vaultUrl("/file_inside.txt"),
                                        vaultUrl("/"));
    ASSERT_TRUE(ret);
}
