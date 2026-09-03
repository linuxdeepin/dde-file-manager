// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-mount/dmount.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QStorageInfo>
#include <QRegularExpression>

#include <dconfig.h>

using namespace dfmplugin_diskenc;

class EncryptUtilsImpl : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ========== recovery_key_utils::formatRecoveryKey ==========

TEST_F(EncryptUtilsImpl, FormatRecoveryKey_Empty)
{
    EXPECT_TRUE(recovery_key_utils::formatRecoveryKey("").isEmpty());
}

TEST_F(EncryptUtilsImpl, FormatRecoveryKey_NoDash)
{
    QString raw = "123456789012345678901234";
    QString formatted = recovery_key_utils::formatRecoveryKey(raw);
    EXPECT_EQ(formatted, "123456-789012-345678-901234");
}

TEST_F(EncryptUtilsImpl, FormatRecoveryKey_WithDashes)
{
    QString raw = "12-34-56-78-90-12-34-56-78-90-12-34";
    QString formatted = recovery_key_utils::formatRecoveryKey(raw);
    EXPECT_EQ(formatted, "123456-789012-345678-901234");
}

TEST_F(EncryptUtilsImpl, FormatRecoveryKey_Truncate)
{
    QString raw = "123456789012345678901234567890";
    QString formatted = recovery_key_utils::formatRecoveryKey(raw);
    EXPECT_EQ(formatted.length(), 27);   // 24 chars + 3 dashes
    EXPECT_EQ(formatted, "123456-789012-345678-901234");
}

TEST_F(EncryptUtilsImpl, FormatRecoveryKey_Short)
{
    QString raw = "1234567890";
    QString formatted = recovery_key_utils::formatRecoveryKey(raw);
    EXPECT_EQ(formatted, "123456-7890");
}

// ========== recovery_key_utils::validateExportPath ==========

TEST_F(EncryptUtilsImpl, ValidateExportPath_Empty)
{
    QString msg;
    EXPECT_FALSE(recovery_key_utils::validateExportPath("", "", &msg));
    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(EncryptUtilsImpl, ValidateExportPath_NotExists)
{
    QString msg;
    EXPECT_FALSE(recovery_key_utils::validateExportPath("/nonexistent/path/for/test", "", &msg));
    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(EncryptUtilsImpl, ValidateExportPath_ValidNoTarget)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    QString msg;
    EXPECT_TRUE(recovery_key_utils::validateExportPath(dir.path(), "", &msg));
    EXPECT_TRUE(msg.isEmpty());
}

TEST_F(EncryptUtilsImpl, ValidateExportPath_SymlinkRejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    QString linkPath = dir.path() + "/link";
    QFile::link(dir.path(), linkPath);

    QString msg;
    EXPECT_FALSE(recovery_key_utils::validateExportPath(linkPath, "", &msg));
    EXPECT_FALSE(msg.isEmpty());
}

// ========== dialog_utils::isWayland ==========

TEST_F(EncryptUtilsImpl, IsWayland_X11)
{
    stub.set_lamda(&QApplication::platformName, []() -> QString {
        __DBG_STUB_INVOKE__
        return "xcb";
    });
    EXPECT_FALSE(dialog_utils::isWayland());
}

TEST_F(EncryptUtilsImpl, IsWayland_Wayland)
{
    stub.set_lamda(&QApplication::platformName, []() -> QString {
        __DBG_STUB_INVOKE__
        return "wayland";
    });
    EXPECT_TRUE(dialog_utils::isWayland());
}

// ========== tpm_passphrase_utils::getGlobalTPMConfigPath ==========

TEST_F(EncryptUtilsImpl, GetGlobalTPMConfigPath_NotEmpty)
{
    QString path = tpm_passphrase_utils::getGlobalTPMConfigPath();
    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(QDir(path).exists());
}

// ========== tpm_passphrase_utils::tpmSupportInterAlgo / tpmSupportSMAlgo ==========

TEST_F(EncryptUtilsImpl, TpmSupportInterAlgo_AllTrue)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = true;
        return 0;
    });
    EXPECT_TRUE(tpm_passphrase_utils::tpmSupportInterAlgo());
}

TEST_F(EncryptUtilsImpl, TpmSupportInterAlgo_RSAFalse)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &algo, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = (algo != "rsa");
        return 0;
    });
    EXPECT_FALSE(tpm_passphrase_utils::tpmSupportInterAlgo());
}

TEST_F(EncryptUtilsImpl, TpmSupportSMAlgo_AllTrue)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = true;
        return 0;
    });
    EXPECT_TRUE(tpm_passphrase_utils::tpmSupportSMAlgo());
}

TEST_F(EncryptUtilsImpl, TpmSupportSMAlgo_SM3False)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &algo, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = (algo != "sm3");
        return 0;
    });
    EXPECT_FALSE(tpm_passphrase_utils::tpmSupportSMAlgo());
}

// ========== tpm_passphrase_utils::getAlgorithm ==========

TEST_F(EncryptUtilsImpl, GetAlgorithm_Inter)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = true;
        return 0;
    });

    QString sessionHash, sessionKey, primaryHash, primaryKey, minorHash, minorKey, pcr, pcrbank;
    EXPECT_TRUE(tpm_passphrase_utils::getAlgorithm(&sessionHash, &sessionKey, &primaryHash, &primaryKey,
                                                   &minorHash, &minorKey, &pcr, &pcrbank));
    EXPECT_EQ(sessionHash, kTPMSessionHashAlgo);
}

TEST_F(EncryptUtilsImpl, GetAlgorithm_SM)
{
    bool first = true;
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [&first](const QString &algo, bool *support) -> int {
        __DBG_STUB_INVOKE__
        if (algo == "rsa" || algo == "aes" || algo == "sha256")
            *support = false;
        else
            *support = true;
        return 0;
    });

    QString sessionHash, sessionKey, primaryHash, primaryKey, minorHash, minorKey, pcr, pcrbank;
    EXPECT_TRUE(tpm_passphrase_utils::getAlgorithm(&sessionHash, &sessionKey, &primaryHash, &primaryKey,
                                                   &minorHash, &minorKey, &pcr, &pcrbank));
    EXPECT_EQ(sessionHash, kTCMSessionHashAlgo);
}

TEST_F(EncryptUtilsImpl, GetAlgorithm_NoSupport)
{
    stub.set_lamda(&tpm_utils::isSupportAlgoByTPM, [](const QString &, bool *support) -> int {
        __DBG_STUB_INVOKE__
        *support = false;
        return 0;
    });

    QString sessionHash, sessionKey, primaryHash, primaryKey, minorHash, minorKey, pcr, pcrbank;
    EXPECT_FALSE(tpm_passphrase_utils::getAlgorithm(&sessionHash, &sessionKey, &primaryHash, &primaryKey,
                                                    &minorHash, &minorKey, &pcr, &pcrbank));
}

// ========== device_utils::resolveEntryBlockDevPath ==========

TEST_F(EncryptUtilsImpl, ResolveEntryBlockDevPath_Empty)
{
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath("").isEmpty());
}

TEST_F(EncryptUtilsImpl, ResolveEntryBlockDevPath_InvalidName)
{
    EXPECT_TRUE(device_utils::resolveEntryBlockDevPath("/dev/../../etc/passwd").isEmpty());
}

TEST_F(EncryptUtilsImpl, ResolveEntryBlockDevPath_NoHolders)
{
    using namespace dfmmount;

    stub.set_lamda(&DDeviceManager::instance, []() -> DDeviceManager * {
        __DBG_STUB_INVOKE__
        static DDeviceManager mgr;
        return &mgr;
    });

    stub.set_lamda(&DDeviceManager::getRegisteredMonitor, [](DDeviceManager *, DeviceType) -> QSharedPointer<dfmmount::DDeviceMonitor> {
        __DBG_STUB_INVOKE__
        return QSharedPointer<dfmmount::DDeviceMonitor>();
    });

    QString result = device_utils::resolveEntryBlockDevPath("/dev/nvme0n1p5");
    EXPECT_EQ(result, "nvme0n1p5.blockdev");
}

// ========== config_utils ==========

TEST_F(EncryptUtilsImpl, ExportKeyEnabled_Stub)
{
    auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
    auto fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
    stub.set_lamda(createFunc, [&](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
        __DBG_STUB_INVOKE__
        return fakeCfg;
    });
    stub.set_lamda(&Dtk::Core::DConfig::value, [](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(config_utils::exportKeyEnabled());
}

TEST_F(EncryptUtilsImpl, CipherType_Stub)
{
    auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
    auto fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
    stub.set_lamda(createFunc, [&](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
        __DBG_STUB_INVOKE__
        return fakeCfg;
    });
    stub.set_lamda(&Dtk::Core::DConfig::value, [](Dtk::Core::DConfig *, const QString &key, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == "encryptAlgorithm") return QString("sm4");
        return QVariant();
    });

    EXPECT_EQ(config_utils::cipherType(), "sm4");
}

TEST_F(EncryptUtilsImpl, CipherType_Fallback)
{
    auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
    auto fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
    stub.set_lamda(createFunc, [&](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
        __DBG_STUB_INVOKE__
        return fakeCfg;
    });
    stub.set_lamda(&Dtk::Core::DConfig::value, [](Dtk::Core::DConfig *, const QString &key, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == "encryptAlgorithm") return QString("unsupported");
        return QVariant();
    });

    EXPECT_EQ(config_utils::cipherType(), "aes");
}
