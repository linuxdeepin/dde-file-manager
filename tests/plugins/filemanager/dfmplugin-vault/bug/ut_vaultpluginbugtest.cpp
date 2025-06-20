// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/unlockview/retrievepasswordview.h"
#include "views/createvaultview/vaultactivesetunlockmethodview.h"
#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/fileencrypthandle.h"
#include "utils/fileencrypthandle_p.h"
#include "fileutils/vaultfileinfo.h"

#include <gtest/gtest.h>

#include <dfm-base/utils/filestatisticsjob.h>
#include <dfm-base/utils/private/filestatisticsjob_p.h>
#include <dfm-base/base/schemefactory.h>

#include <DComboBox>

#include <QStandardPaths>
#include <QProcess>
#include <QDBusConnection>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultPluginBugTest, bug_193745_WindowNoStaysOnTop)
{
    RetrievePasswordView view;
    view.onComboBoxIndex(1);

    Qt::WindowFlags flags = view.fileDialog->windowFlags();
    EXPECT_TRUE(flags & Qt::WindowStaysOnTopHint);
}

TEST(UT_VaultPluginBugTest, bug_178305_NotRegisterService)
{
    bool isRegisterService { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::pluginServiceRegister, [ &isRegisterService ] { __DBG_STUB_INVOKE__ isRegisterService = true; });
    typedef bool(QDBusConnection::*FuncType)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<FuncType>(&QDBusConnection::connect), []{
        return true;
    });
    typedef bool(QDBusConnection::*FuncType2)(const QString &, const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<FuncType2>(&QDBusConnection::connect), []{
        return true;
    });

    Vault plugin;
    plugin.initialize();
    EXPECT_FALSE(isRegisterService);
}

TEST(UT_VaultPluginBugTest, bug_177631_NotHaveTransparentEncryptionFunction)
{
    VaultActiveSetUnlockMethodView view;
    int count = view.typeCombo->count();
    EXPECT_TRUE(count > 1);
    if (count > 1) {
        EXPECT_TRUE(view.typeCombo->itemText(1) == VaultActiveSetUnlockMethodView::tr("Transparent encryption"));
    }
}

TEST(UT_VaultPluginBugTest, bug_172923_ErrorPasswordReturnTrue)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultConfig::set, [] { __DBG_STUB_INVOKE__ });
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), [] { __DBG_STUB_INVOKE__ return QVariant("new"); });
    typedef bool(*FuncType2)(QIODevice::OpenMode);
    stub.set_lamda((FuncType2)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open), [] { __DBG_STUB_INVOKE__ return false; });

    OperatorCenter::getInstance()->savePasswordAndPasswordHint("123456", "password");
    QString cipher;
    bool b = OperatorCenter::getInstance()->checkPassword("123", cipher);
    EXPECT_FALSE(b);
}

TEST(UT_VaultPluginBugTest, bug_172877_CheckCryfsSettings)
{
    FileEncryptHandlerPrivate::CryfsVersionInfo cryfsVer(0, 10, 0);
    stub_ext::StubExt stub;
    stub.set_lamda(&QStandardPaths::findExecutable, [] { return "/uttest/bug_172877_CheckCryfsSettings/executable"; });
    stub.set_lamda(&FileEncryptHandlerPrivate::versionString, [ &cryfsVer ] {return cryfsVer; });

    FileEncryptHandle::instance()->d->runVaultProcess("/uttest1", "/uttest2", "123456");
    QStringList args = FileEncryptHandle::instance()->d->process->arguments();
    EXPECT_TRUE(args.contains("--allow-replaced-filesystem"));
}

TEST(UT_VaultPluginBugTest, bug_164615_countProcCrash)
{
    QQueue<QUrl> list;
    FileStatisticsJob job;
    EXPECT_NO_FATAL_FAILURE(job.d->processFile(QUrl("file:///proc"), false, list));
    FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl("file:///proc"), Global::CreateFileInfoType::kCreateFileInfoSync);
    EXPECT_TRUE(!info);
}

TEST(UT_VaultPluginBugTest, bug_144787_CheckCryfs)
{
#ifdef COMPILE_ON_V23
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    EXPECT_FALSE(cryfsBinary.isEmpty());
#endif
}

TEST(UT_VaultPluginBugTest, bug_200185_CheckProxyChange)
{
    VaultFileInfo info(QUrl("dfmvault:///"));
    FileInfoPointer oldprox = info.proxy;
    info.refresh();
    EXPECT_TRUE(oldprox == info.proxy);
}

TEST(UT_VaultPluginBugTest, bug_199947_CacheVaultFileInfo)
{
    bool isCache { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&isCache](const QUrl &url,
                                                              const Global::CreateFileInfoType type,
                                                              QString *errorString){
        if (type == Global::CreateFileInfoType::kCreateFileInfoSyncAndCache)
            isCache = true;
        return nullptr;
    });
    stub.set_lamda(&ProxyFileInfo::setProxy, []{});

    VaultFileInfo info(QUrl("dfmvault:///"));
    EXPECT_TRUE(isCache);
}

TEST(UT_VaultPluginBugTest, bug_202109_VaultStateError)
{
   VaultState state =  FileEncryptHandle::instance()->state("");
    EXPECT_TRUE(state == VaultState::kUnknow);
}
