// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Aggregated coverage for small vault core pieces: VaultEntryFileEntity,
// VaultUtils, ServiceManager ctor, VaultFileHelper job callbacks,
// VaultEventCaller publish helpers, Vault plugin start/bindWindows and
// VaultVisibleManager sidebar/computer registration helpers. All framework
// slot-channel targets are not loaded in this binary, so pushes are no-ops.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QObject>
#include <QPointer>
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

#include "stubext.h"

#include "vault.h"
#include "utils/vaultentryfileentity.h"
#include "utils/vaultutils.h"
#include "utils/servicemanager.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/vaultfilehelper.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/fileencrypthandle.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-framework/dpf.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

// Static catcher subscribed once; kept alive for the whole process so late
// publishes of the same global events never hit a dangling receiver.
class VaultSigCatcher : public QObject
{
    Q_OBJECT
public:
    QUrl lastOpenWindowUrl;
    QUrl lastOpenTabUrl;
    quint64 lastOpenTabWinId { 0 };
    int openWindowCount { 0 };
    int openTabCount { 0 };

public slots:
    void onOpenWindow(const QUrl &url)
    {
        lastOpenWindowUrl = url;
        ++openWindowCount;
    }
    void onOpenTab(const quint64 &winId, const QUrl &url)
    {
        lastOpenTabWinId = winId;
        lastOpenTabUrl = url;
        ++openTabCount;
    }
};

static VaultSigCatcher *gCatcher = nullptr;
static void ensureCatcher()
{
    static bool done = false;
    if (!done) {
        gCatcher = new VaultSigCatcher();
        dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewWindow, gCatcher, &VaultSigCatcher::onOpenWindow);
        dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewTab, gCatcher, &VaultSigCatcher::onOpenTab);
        done = true;
    }
}

class VaultCoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ensureCatcher();
        baseWin = gCatcher->openWindowCount;
        baseTab = gCatcher->openTabCount;
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    int baseWin = 0;
    int baseTab = 0;
};

// --- VaultEntryFileEntity ---

TEST_F(VaultCoreTest, EntryEntity_MetaGetters_ReturnExpectedDefaults)
{
    VaultEntryFileEntity entity;
    EXPECT_EQ(entity.displayName(), QObject::tr("File Vault"));
    EXPECT_TRUE(entity.exists());
    EXPECT_FALSE(entity.showProgress());
    EXPECT_FALSE(entity.showUsageSize());
}

TEST_F(VaultCoreTest, EntryEntity_Order_IsCustomPlusOne)
{
    VaultEntryFileEntity entity;
    int expected = static_cast<int>(DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderCustom) + 1;
    EXPECT_EQ(static_cast<int>(entity.order()), expected);
}

TEST_F(VaultCoreTest, EntryEntity_TargetUrl_IsVaultRoot)
{
    VaultEntryFileEntity entity;
    EXPECT_EQ(entity.targetUrl(), VaultHelper::instance()->rootUrl());
}

TEST_F(VaultCoreTest, EntryEntity_TotalSize_InitialZero)
{
    VaultEntryFileEntity entity;
    EXPECT_EQ(entity.sizeTotal(), quint64(0));
}

TEST_F(VaultCoreTest, EntryEntity_Refresh_NoOp)
{
    VaultEntryFileEntity entity;
    entity.refresh();
    EXPECT_EQ(entity.sizeTotal(), quint64(0));
}

TEST_F(VaultCoreTest, EntryEntity_ShowTotalSize_LockedVault_ReturnsFalse)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState {
                       return VaultState::kEncrypted;
                   });
    VaultEntryFileEntity entity;
    EXPECT_FALSE(entity.showTotalSize());
}

TEST_F(VaultCoreTest, EntryEntity_SizeChangeSlots_UpdateTotals)
{
    tempDir = std::make_unique<QTemporaryDir>();
    ASSERT_TRUE(tempDir->isValid());
    QFile f(tempDir->path() + "/payload.bin");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("12345");
    f.close();

    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState {
                       return VaultState::kUnlocked;
                   });
    stub.set_lamda(&PathManager::makeVaultLocalPath, [this](const QString &, const QString &) -> QString {
        return tempDir->path();
    });
    stub.set_lamda(&VaultHelper::vaultToLocalUrl, [this](const QUrl &url) -> QUrl {
        // sourceRootUrl().path() already is the temp dir; child paths are appended
        if (url.path().startsWith(tempDir->path()))
            return QUrl::fromLocalFile(url.path());
        return QUrl::fromLocalFile(tempDir->path() + url.path());
    });

    VaultEntryFileEntity entity;
    ASSERT_TRUE(entity.showTotalSize());   // unlocked -> starts size calculation
    QTest::qWait(300);                     // first scan settles

    ASSERT_TRUE(entity.showTotalSize());   // restart: showSizeState is true again
    DFMBASE_NAMESPACE::FileScanner::ScanResult result;
    result.totalSize = 4321;
    entity.slotFileDirSizeChange(result);   // synchronous direct call
    entity.slotFinishedThread();            // commits totalchange into vaultTotal
    EXPECT_EQ(entity.sizeTotal(), quint64(4321));
}

// --- VaultUtils ---

TEST_F(VaultCoreTest, VaultUtils_Instance_Singleton)
{
    ASSERT_NE(&VaultUtils::instance(), nullptr);
    EXPECT_EQ(&VaultUtils::instance(), &VaultUtils::instance());
}

TEST_F(VaultCoreTest, VaultUtils_AuthorityDenied_EmitsFalse)
{
    QSignalSpy spy(&VaultUtils::instance(), &VaultUtils::resultOfAuthority);
    VaultUtils::instance().slotCheckAuthorizationFinished(PolkitQt1::Authority::No);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

TEST_F(VaultCoreTest, VaultUtils_AuthorityGranted_EmitsTrue)
{
    QSignalSpy spy(&VaultUtils::instance(), &VaultUtils::resultOfAuthority);
    VaultUtils::instance().slotCheckAuthorizationFinished(PolkitQt1::Authority::Yes);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.at(0).at(0).toBool());
}

TEST_F(VaultCoreTest, VaultUtils_ShowAuthorityDialog_ChecksAuthorization)
{
    bool checked = false;
    stub.set_lamda(&PolkitQt1::Authority::checkAuthorization,
                   [&checked](PolkitQt1::Authority *, const QString &,
                              const PolkitQt1::Subject &,
                              PolkitQt1::Authority::AuthorizationFlags) {
                       checked = true;
                   });
    VaultUtils::instance().showAuthorityDialog("org.ut.vault.action");
    EXPECT_TRUE(checked);
}

// --- ServiceManager ---

TEST_F(VaultCoreTest, ServiceManager_ConstructWithParent_ParentOwns)
{
    QObject *owner = new QObject();
    QPointer<ServiceManager> mgr = new ServiceManager(owner);
    ASSERT_FALSE(mgr.isNull());
    delete owner;
    EXPECT_TRUE(mgr.isNull());
}

// --- VaultFileHelper job callbacks ---

TEST_F(VaultCoreTest, CallBackFunction_NullJobHandle_NoCursorOverride)
{
    AbstractJobHandler::CallbackArgus args(
            new QMap<AbstractJobHandler::CallbackKey, QVariant>());
    args->insert(AbstractJobHandler::CallbackKey::kJobHandle,
                 QVariant::fromValue(JobHandlePointer()));
    VaultFileHelper::instance()->callBackFunction(args);
    SUCCEED();
}

TEST_F(VaultCoreTest, CallBackFunction_ValidJobHandle_ConnectsFinishNotify)
{
    JobHandlePointer handle(new AbstractJobHandler());
    AbstractJobHandler::CallbackArgus args(
            new QMap<AbstractJobHandler::CallbackKey, QVariant>());
    args->insert(AbstractJobHandler::CallbackKey::kJobHandle,
                 QVariant::fromValue(handle));
    VaultFileHelper::instance()->callBackFunction(args);
    EXPECT_NE(handle.data(), nullptr);

    // emulate job finish -> handleFinishedNotify runs and restores cursor
    VaultFileHelper::instance()->handleFinishedNotify(JobInfoPointer());
    SUCCEED();
}

TEST_F(VaultCoreTest, HandleFinishedNotify_NullSender_NoCrash)
{
    VaultFileHelper::instance()->handleFinishedNotify(JobInfoPointer());
    SUCCEED();
}

// --- VaultEventCaller ---

TEST_F(VaultCoreTest, SendOpenWindow_PublishesVaultUrl)
{
    QUrl url("dfmvault:///somewhere");
    VaultEventCaller::sendOpenWindow(url);
    EXPECT_EQ(gCatcher->openWindowCount - baseWin, 1);
    EXPECT_EQ(gCatcher->lastOpenWindowUrl, url);
}

TEST_F(VaultCoreTest, SendOpenTab_PublishesWindowAndUrl)
{
    QUrl url("dfmvault:///tabbed");
    VaultEventCaller::sendOpenTab(99, url);
    EXPECT_EQ(gCatcher->openTabCount - baseTab, 1);
    EXPECT_EQ(gCatcher->lastOpenTabWinId, quint64(99));
    EXPECT_EQ(gCatcher->lastOpenTabUrl, url);
}

TEST_F(VaultCoreTest, SendVaultProperty_PluginNotLoaded_ReturnsQuietly)
{
    VaultEventCaller::sendVaultProperty(QUrl("dfmvault:///"));
    SUCCEED();
}

// --- Vault plugin lifecycle ---

TEST_F(VaultCoreTest, VaultStart_RegistersServices_ReturnsTrue)
{
    Vault vault;
    EXPECT_TRUE(vault.start());
}

TEST_F(VaultCoreTest, VaultBindWindows_NoWindows_NoCrash)
{
    Vault vault;
    vault.bindWindows();
    SUCCEED();
}

TEST_F(VaultCoreTest, VisibleManager_OnUnknownWindow_Ignored)
{
    VaultVisibleManager::instance()->onWindowOpened(60000);
    SUCCEED();
}

TEST_F(VaultCoreTest, VisibleManager_SidebarHelpers_RunQuietly)
{
    VaultVisibleManager::instance()->updateSideBarVaultItem();
    VaultVisibleManager::instance()->removeSideBarVaultItem();
    VaultVisibleManager::instance()->removeComputerVaultItem();
    SUCCEED();
}

#include "test_vaultcore.moc"
