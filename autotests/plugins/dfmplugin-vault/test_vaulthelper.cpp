// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>

#include "stubext.h"

#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/vaultautolock.h"
#include "dbus/vaultdbusutils.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/event/event.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class VaultHelperImpl : public testing::Test
{
public:
    void SetUp() override
    {
        VaultHelper::instance()->removeWinID(42);
        VaultHelper::instance()->removeWinID(43);
    }

    void TearDown() override
    {
        stub.clear();
        VaultHelper::instance()->removeWinID(42);
        VaultHelper::instance()->removeWinID(43);
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(VaultHelperImpl, Scheme)
{
    VaultHelper *h = VaultHelper::instance();
    EXPECT_EQ(h->scheme(), QString("dfmvault"));
}

TEST_F(VaultHelperImpl, RootUrl)
{
    QUrl url = VaultHelper::instance()->rootUrl();
    EXPECT_EQ(url.scheme(), QString("dfmvault"));
    EXPECT_EQ(url.path(), QString("/"));
}

TEST_F(VaultHelperImpl, SourceRootUrl)
{
    stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
        return QString("/tmp/vault_unlocked");
    });

    QUrl url = VaultHelper::instance()->sourceRootUrl();
    EXPECT_EQ(url.scheme(), QString("dfmvault"));
    EXPECT_EQ(url.path(), QString("/tmp/vault_unlocked"));
}

TEST_F(VaultHelperImpl, SourceRootUrlWithSlash)
{
    stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
        return QString("/tmp/vault_unlocked");
    });
    stub.set_lamda(&PathManager::addPathSlash, [](const QString &) -> QString {
        return QString("/tmp/vault_unlocked/");
    });

    QUrl url = VaultHelper::instance()->sourceRootUrlWithSlash();
    EXPECT_EQ(url.path(), QString("/tmp/vault_unlocked/"));
}

TEST_F(VaultHelperImpl, PathToVaultVirtualUrl_MatchesSourceRoot)
{
    QUrl source = VaultHelper::instance()->sourceRootUrl();
    QString local = source.path();
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(local);
    EXPECT_EQ(virtualUrl.path(), QString("/"));
    EXPECT_EQ(virtualUrl.scheme(), QString("dfmvault"));
}

TEST_F(VaultHelperImpl, PathToVaultVirtualUrl_ChildPath)
{
    QUrl source = VaultHelper::instance()->sourceRootUrl();
    QString child = source.path() + "/foo/bar.txt";
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(child);
    EXPECT_EQ(virtualUrl.path(), QString("/foo/bar.txt"));
}

TEST_F(VaultHelperImpl, PathToVaultVirtualUrl_NoMatch)
{
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl("/tmp/other");
    EXPECT_TRUE(virtualUrl.isEmpty());
}

TEST_F(VaultHelperImpl, VaultToLocalUrl_FileScheme)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/foo.txt");
    QUrl local = VaultHelper::vaultToLocalUrl(fileUrl);
    EXPECT_EQ(local, fileUrl);
}

TEST_F(VaultHelperImpl, VaultToLocalUrl_WrongScheme)
{
    QUrl wrong;
    wrong.setScheme("http");
    wrong.setPath("/tmp/foo.txt");
    QUrl local = VaultHelper::vaultToLocalUrl(wrong);
    EXPECT_TRUE(local.isEmpty());
}

TEST_F(VaultHelperImpl, IsVaultFile_ByScheme)
{
    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    EXPECT_TRUE(VaultHelper::isVaultFile(url));
}

TEST_F(VaultHelperImpl, UrlsToLocal_NullOutput)
{
    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    EXPECT_FALSE(VaultHelper::instance()->urlsToLocal({ url }, nullptr));
}

TEST_F(VaultHelperImpl, UrlsToLocal_FileSchemePath)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/foo.txt");
    QList<QUrl> out;
    // The source URL is not a vault URL, so it returns false.
    EXPECT_FALSE(VaultHelper::instance()->urlsToLocal({ fileUrl }, &out));
}

TEST_F(VaultHelperImpl, CurrentAndWinIDs)
{
    VaultHelper *h = VaultHelper::instance();
    h->appendWinID(42);
    EXPECT_EQ(h->currentWindowId(), 42u);
    h->appendWinID(43);
    EXPECT_EQ(h->currentWindowId(), 43u);
    h->removeWinID(42);
    EXPECT_EQ(h->currentWindowId(), 43u);
    h->removeWinID(43);
    // removeWinID does not modify currentWinID; it only removes from the list.
    EXPECT_TRUE(h->currentWindowId() == 43u);
}

TEST_F(VaultHelperImpl, State_DelegatesToFileEncryptHandle)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kUnlocked; });

    EXPECT_EQ(VaultHelper::instance()->state("/tmp"), kUnlocked);
}

TEST_F(VaultHelperImpl, UpdateState)
{
    using UpdateFunc = bool (FileEncryptHandle::*)(VaultState);
    stub.set_lamda(static_cast<UpdateFunc>(&FileEncryptHandle::updateState),
                   [](FileEncryptHandle *, VaultState) -> bool { return true; });

    EXPECT_TRUE(VaultHelper::instance()->updateState(kUnlocked));
}

TEST_F(VaultHelperImpl, EnableUnlockVault_ConfigTrue)
{
    using ValueFunc = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<ValueFunc>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(true);
                   });

    EXPECT_TRUE(VaultHelper::instance()->enableUnlockVault());
}

TEST_F(VaultHelperImpl, EnableUnlockVault_ConfigFalseNoNet)
{
    using ValueFunc = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<ValueFunc>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(false);
                   });
    stub.set_lamda(&VaultDBusUtils::isFullConnectInternet, []() -> bool { return false; });

    EXPECT_TRUE(VaultHelper::instance()->enableUnlockVault());
}

TEST_F(VaultHelperImpl, EnableUnlockVault_ConfigFalseWithNet)
{
    using ValueFunc = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<ValueFunc>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(false);
                   });
    stub.set_lamda(&VaultDBusUtils::isFullConnectInternet, []() -> bool { return true; });

    EXPECT_FALSE(VaultHelper::instance()->enableUnlockVault());
}

TEST_F(VaultHelperImpl, SlotLockVault_EmitsLocked)
{
    VaultHelper *h = VaultHelper::instance();
    h->appendWinID(42);
    QSignalSpy spy(h, &VaultHelper::sigLocked);

    h->slotlockVault(0);
    EXPECT_EQ(spy.count(), 1);
    h->removeWinID(42);
}

TEST_F(VaultHelperImpl, GetVaultVersion)
{
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyVersion, QVariant("1050"));

    EXPECT_TRUE(VaultHelper::instance()->getVaultVersion());
}

TEST_F(VaultHelperImpl, CreateVault_DelegatesToFileEncryptHandle)
{
    QString password = "password";
    using CreateFunc = void (FileEncryptHandle::*)(const QString &, const QString &, const QString &, EncryptType, int);
    bool called = false;
    stub.set_lamda(static_cast<CreateFunc>(&FileEncryptHandle::createVault),
                   [&called](FileEncryptHandle *, const QString &, const QString &, const QString &, EncryptType, int) {
                       called = true;
                   });
    stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
        return QString("/tmp/vault");
    });

    VaultHelper::instance()->createVault(password);
    EXPECT_TRUE(called);
}

TEST_F(VaultHelperImpl, UnlockVault_DelegatesToFileEncryptHandle)
{
    using UnlockFunc = bool (FileEncryptHandle::*)(const QString &, const QString &, const QString &);
    stub.set_lamda(static_cast<UnlockFunc>(&FileEncryptHandle::unlockVault),
                   [](FileEncryptHandle *, const QString &, const QString &, const QString &) -> bool { return true; });
    stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
        return QString("/tmp/vault");
    });

    EXPECT_TRUE(VaultHelper::instance()->unlockVault("password"));
}

TEST_F(VaultHelperImpl, LockVault_DelegatesToFileEncryptHandle)
{
    using LockFunc = bool (FileEncryptHandle::*)(QString, bool);
    stub.set_lamda(static_cast<LockFunc>(&FileEncryptHandle::lockVault),
                   [](FileEncryptHandle *, QString, bool) -> bool { return true; });

    EXPECT_TRUE(VaultHelper::instance()->lockVault(false));
}

TEST_F(VaultHelperImpl, OpenWidWindow)
{
    stub.set_lamda(&VaultEventCaller::sendItemActived,
                   [](quint64, const QUrl &) {});

    VaultHelper::instance()->appendWinID(100);
    VaultHelper::instance()->openWidWindow(100, QUrl("dfmvault:///"));
    SUCCEED();
}

TEST_F(VaultHelperImpl, DefaultCdAction)
{
    stub.set_lamda(&VaultEventCaller::sendItemActived,
                   [](quint64, const QUrl &) {});

    VaultHelper::instance()->defaultCdAction(100, QUrl("dfmvault:///"));
    SUCCEED();
}

TEST_F(VaultHelperImpl, ShowInProgressDialog_Busy)
{
    using DialogFunc = void (DialogManager::*)(const QString &, const QString &);
    bool called = false;
    stub.set_lamda(VADDR(DialogManager, showErrorDialog),
                   [&called](DialogManager *, const QString &, const QString &) { called = true; });

    VaultHelper::instance()->showInProgressDailog("Device or resource busy");
    EXPECT_TRUE(called);
}

// ---------------------------------------------------------------------------
// PMS sev-2 regression additions (vault helper)
// ---------------------------------------------------------------------------
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QStandardPaths>

#include "utils/vaultdefine.h"
#include <dfm-io/dfmio_utils.h>

// PMS:125703 pathToVaultVirtualUrl used to parse the local path with the
// QUrl(QString) ctor, so characters like '#' were split off as a fragment and
// the produced dfmvault url was broken; drag&drop aborted and the preview
// dialog crashed on close. The url must be built with setPath/setScheme and
// survive a full round trip back to the local path.
TEST_F(VaultHelperImpl, BUG125703_VirtualUrlSpecialCharsRoundTrip)
{
    stub.set_lamda(&PathManager::makeVaultLocalPath,
                   [](const QString &, const QString &) -> QString {
                       return QString("/tmp/ut125703_vault");
                   });
    stub.set_lamda(&PathManager::addPathSlash,
                   [](const QString &path) -> QString {
                       return path.endsWith('/') ? path : path + '/';
                   });

    VaultHelper *h = VaultHelper::instance();
    const QString fileName = QStringLiteral("ut drag #1 100%&视频?.txt");
    const QString localPath = QStringLiteral("/tmp/ut125703_vault/") + fileName;

    QUrl virtualUrl = h->pathToVaultVirtualUrl(localPath);
    EXPECT_TRUE(virtualUrl.isValid());
    EXPECT_EQ(virtualUrl.scheme(), QString("dfmvault"));
    // the full file name must live in the path, nothing may leak into fragment/query
    EXPECT_EQ(virtualUrl.path(), QStringLiteral("/") + fileName);
    EXPECT_TRUE(virtualUrl.fragment().isEmpty());
    EXPECT_TRUE(virtualUrl.query().isEmpty());

    // round trip: virtual -> local must restore the exact local path
    QUrl back = VaultHelper::vaultToLocalUrl(virtualUrl);
    EXPECT_EQ(back.toLocalFile(), localPath);
}

// PMS:177631 a transparent-encryption vault must unlock straight from the
// keyring password instead of opening the unlock pages dialog.
TEST_F(VaultHelperImpl, BUG177631_TransparentEncryptionUnlocksFromKeyring)
{
    using CfgGetFunc = QVariant (VaultConfig::*)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<CfgGetFunc>(&VaultConfig::get),
                   [](VaultConfig *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(QString(kConfigValueMethodTransparent));
                   });
    stub.set_lamda(ADDR(OperatorCenter, passwordFromKeyring),
                   [](OperatorCenter *) -> QString {
                       return QStringLiteral("ut-keyring-password");
                   });
    stub.set_lamda(&PathManager::createVaultMountDir,
                   [](const QString &) -> bool { return true; });

    int unlockCalls = 0;
    QString unlockedWith;
    stub.set_lamda(ADDR(VaultHelper, unlockVault),
                   [&unlockCalls, &unlockedWith](VaultHelper *, const QString &password) -> bool {
                       ++unlockCalls;
                       unlockedWith = password;
                       return true;
                   });
    stub.set_lamda(ADDR(VaultHelper, defaultCdAction),
                   [](VaultHelper *, const quint64, const QUrl &) {
                   });
    stub.set_lamda(&VaultHelper::recordTime,
                   [](const QString &, const QString &) {
                   });

    VaultHelper::instance()->unlockVaultDialog();

    EXPECT_EQ(unlockCalls, 1);
    EXPECT_EQ(unlockedWith, QStringLiteral("ut-keyring-password"));
}

// PMS:202109 FileEncryptHandle::state("") must report kUnknow (never kNotExisted),
// otherwise the sidebar falls back into the "create vault" flow after password
// retrieval and pops the creation wizard.
TEST_F(VaultHelperImpl, BUG202109_EmptyBaseDirReportsUnknownState)
{
    EXPECT_EQ(FileEncryptHandle::instance()->state(QString()), VaultState::kUnknow);
    EXPECT_EQ(FileEncryptHandle::instance()->state(QString(), false), VaultState::kUnknow);
    EXPECT_EQ(VaultHelper::instance()->state(QString()), VaultState::kUnknow);
}

// PMS:140787 state() must decide the vault state from DFMIO::DFMUtils::fsTypeFromUrl
// (mount-aware), not from QStorageInfo; the unlock path is queried by url and a
// fuse.cryfs fs type reports kUnlocked while any other fs type stays kEncrypted.
TEST_F(VaultHelperImpl, BUG140787_UnlockPathFuseTypeDecidesUnlockedState)
{
    QTemporaryDir realUnlockDir;
    ASSERT_TRUE(realUnlockDir.isValid());
    QFile cfg(realUnlockDir.path() + "/cryfs.config");
    ASSERT_TRUE(cfg.open(QIODevice::WriteOnly));
    cfg.write("ut marker");
    cfg.close();

    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return "/usr/bin/cryfs"; });
    const QString unlockDirPath = realUnlockDir.path();
    stub.set_lamda(&PathManager::vaultUnlockPath,
                   [unlockDirPath]() -> QString { return unlockDirPath; });

    QList<QUrl> queried;
    stub.set_lamda(&DFMIO::DFMUtils::fsTypeFromUrl,
                   [&queried](const QUrl &url) -> QString {
                       queried << url;
                       return QStringLiteral("fuse.cryfs");
                   });

    EXPECT_EQ(FileEncryptHandle::instance()->state(realUnlockDir.path(), false), VaultState::kUnlocked);
    ASSERT_FALSE(queried.isEmpty());
    // the fs type must be queried on the unlock path url (mount point), not on storage info
    EXPECT_EQ(queried.first(), QUrl::fromLocalFile(QFileInfo(realUnlockDir.path()).canonicalFilePath()));

    // the same marker file on a non-fuse fs type must NOT report unlocked — the
    // decision must come from fsTypeFromUrl, not from the cryfs.config presence
    queried.clear();
    stub.set_lamda(&DFMIO::DFMUtils::fsTypeFromUrl,
                   [&queried](const QUrl &url) -> QString {
                       queried << url;
                       return QStringLiteral("ext4");
                   });
    EXPECT_EQ(FileEncryptHandle::instance()->state(realUnlockDir.path(), false), VaultState::kEncrypted);
    EXPECT_FALSE(queried.isEmpty());
}
