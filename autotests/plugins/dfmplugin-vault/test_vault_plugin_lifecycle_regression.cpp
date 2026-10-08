// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests around the vault plugin lifecycle pieces.
// Covers: BUG-199339 (Vault::initialize must not register plugin services,
// only Vault::start does), BUG-316335 (key-mode creation must persist the
// encryption configuration before trying to save the password, so a failed
// save still leaves the vault in key mode), BUG-200185 (VaultFileInfo::refresh
// must keep the proxy FileInfo object instead of recreating it), BUG-134877
// (VaultFileInfo::refresh must re-stat through the proxy so fresh file state
// becomes visible).
// No real encryption volume is created or mounted.

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QIcon>

#include "stubext.h"

#include "vault.h"
#include "utils/vaultvisiblemanager.h"

#define private public
#define protected public
#include "views/vaultcreatepage.h"
#undef protected
#undef private

#include "events/vaulteventcaller.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/vaultdefine.h"
#include "views/createvaultview/vaultactivefinishedview.h"

#include "fileutils/vaultfileinfo.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/utils/fileinfohelper.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
const QString kConfigPath = kVaultBasePath + "/" + QString(kVaultConfigFileName);
}   // namespace

// --- BUG:199339 service registration timing ---

class VaultPluginLifecycleTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(VaultVisibleManager, pluginServiceRegister),
                       [](VaultVisibleManager *) {
                           ++sRegisterCount;
                       });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    static int sRegisterCount;
};

int VaultPluginLifecycleTest::sRegisterCount = 0;

// PMS:199339 initialize() must not publish plugin services; start() must do it exactly once
TEST_F(VaultPluginLifecycleTest, BUG199339_InitializeSkipsServiceRegister_StartRegistersOnce)
{
    ASSERT_EQ(sRegisterCount, 0);

    Vault vault;
    vault.initialize();
    // the fix moved pluginServiceRegister out of initialize
    EXPECT_EQ(sRegisterCount, 0);

    ASSERT_TRUE(vault.start());
    EXPECT_EQ(sRegisterCount, 1);
}

// --- BUG:316335 key-mode config is persisted before password save ---

class VaultKeyModePageTest : public testing::Test
{
protected:
    void SetUp() override
    {
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");
        // remove any stale key-mode switches so the test starts clean
        {
            VaultConfig cfg;
            cfg.set(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant());
            cfg.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant());
        }

        stub.set_lamda(&WindowUtils::isWayLand, []() -> bool { return false; });
        stub.set_lamda(&OperatorCenter::createDirAndFile, []() -> Result { return { true, "" }; });
        stub.set_lamda(&VaultHelper::defaultCdAction, [](VaultHelper *, quint64, const QUrl &) {});
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        // the finished widget is not wired up in this flow; keep its notify a no-op
        stub.set_lamda(ADDR(VaultActiveFinishedView, encryptFinished),
                       [](VaultActiveFinishedView *, bool, const QString &) {});
        view = new VaultActiveView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
        QFile::remove(kConfigPath);
        if (QFile::exists(kConfigPath + ".utbak")) {
            QFile::remove(kConfigPath);
            QFile::rename(kConfigPath + ".utbak", kConfigPath);
        }
    }

    stub_ext::StubExt stub;
    VaultActiveView *view = nullptr;
};

// PMS:316335 handleKeyModeEncryption must write the key-mode config BEFORE
// attempting the password save, so even a failed save leaves the vault
// configured for key encryption
TEST_F(VaultKeyModePageTest, BUG316335_KeyModeConfigSavedBeforePasswordSave)
{
    ASSERT_NE(view, nullptr);
    view->encryptInfo.password = QStringLiteral("ut-key-password");
    view->encryptInfo.hint = QStringLiteral("ut-key-hint");

    // the password save fails (e.g. container error) right after the config write
    stub.set_lamda(ADDR(OperatorCenter, savePasswordAndPasswordHint),
                   [](OperatorCenter *, const QString &, const QString &) -> Result {
                       return { false, QStringLiteral("ut-fail") };
                   });

    EXPECT_FALSE(view->handleKeyModeEncryption());

    // regression: the config must already carry the key-mode values
    VaultConfig cfg;
    EXPECT_EQ(cfg.get(kConfigNodeName, kConfigKeyUseUserPassWord).toString(), QString("Yes"));
    EXPECT_EQ(cfg.get(kConfigNodeName, kConfigKeyEncryptionMethod).toString(), QString(kConfigValueMethodKey));
}

// --- BUG:200185 / BUG:134877 VaultFileInfo refresh semantics ---

class VaultFileInfoRegressionTest : public testing::Test
{
public:
    void SetUp() override
    {
        static bool sFileSchemeReady = false;
        if (!sFileSchemeReady) {
            UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, "File");
            UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, "File");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
            InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
            sFileSchemeReady = true;
        }

        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid());
        const QString base = tempDir->path();

        stub.set_lamda(&VaultHelper::vaultToLocalUrl, [base](const QUrl &url) -> QUrl {
            return QUrl::fromLocalFile(base + url.path());
        });
        stub.set_lamda(&PathManager::makeVaultLocalPath, [base](const QString &, const QString &) -> QString {
            return base;
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl vaultUrl(const QString &path) const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath(path);
        return url;
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// PMS:200185 refreshing vault file info must not destroy/recreate the proxy
// FileInfo (the old behaviour crashed the thumbnail while it still used the icon)
TEST_F(VaultFileInfoRegressionTest, BUG200185_RefreshKeepsProxyIdentity)
{
    const QString target = tempDir->path() + "/ut_file.txt";
    {
        QFile f(target);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("ut-content");
        f.close();
    }

    VaultFileInfo info(vaultUrl("/ut_file.txt"));
    ASSERT_TRUE(bool(info.proxy));
    FileInfoPointer oldProxy = info.proxy;

    info.refresh();

    // the very same proxy object must survive the refresh
    EXPECT_TRUE(oldProxy == info.proxy);
    EXPECT_EQ(oldProxy.data(), info.proxy.data());
}

// PMS:134877 refresh must push the re-stat through the proxy (created from the
// proxy url); AsyncFileInfo::refresh is the only path that triggers
TEST_F(VaultFileInfoRegressionTest, BUG134877_RefreshRestatsThroughProxy)
{
    const QString target = tempDir->path() + "/ut_late_file.txt";
    {
        QFile f(target);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("ut-late-content");
        f.close();
    }

    VaultFileInfo info(vaultUrl("/ut_late_file.txt"));
    ASSERT_TRUE(bool(info.proxy));

    static int sProxyRefreshCount = 0;
    sProxyRefreshCount = 0;
    stub.set_lamda(ADDR(FileInfoHelper, fileRefreshAsync),
                   [](FileInfoHelper *, const QSharedPointer<FileInfo>) {
                       ++sProxyRefreshCount;
                   });

    info.refresh();

    // the fix re-stats via proxy->refresh(); before the fix the proxy was never asked
    EXPECT_EQ(sProxyRefreshCount, 1);
}
