// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
#define SERVERPLUGIN_VAULTDAEMON_GLOBAL_H

#define SERVERVAULT_NAMESPCE serverplugin_vaultdaemon

#define SERVERVAULT_BEGIN_NAMESPACE namespace SERVERVAULT_NAMESPCE {
#define SERVERVAULT_END_NAMESPACE }
#define SERVERVAULT_USE_NAMESPACE using namespace SERVERVAULT_NAMESPCE;

#include <dfm-base/dfm_log_defines.h>

#include <QString>
#include <QDir>

SERVERVAULT_BEGIN_NAMESPACE

DFM_LOG_USE_CATEGORY(SERVERVAULT_NAMESPCE)

#ifdef COMPILE_ON_V23
inline constexpr char kAppSessionService[] { "org.deepin.dde.SessionManager1" };
inline constexpr char kAppSessionPath[] { "/org/deepin/dde/SessionManager1" };
#else
inline constexpr char kAppSessionService[] { "com.deepin.SessionManager" };
inline constexpr char kAppSessionPath[] { "/com/deepin/SessionManager" };
#endif

inline const QString kVaultConfigPath(QDir::homePath() + QString("/.config/Vault"));
inline constexpr char kVaultMountDirName[] { "vault_unlocked" };
inline constexpr char kVaultBaseDirName[] { "vault_encrypted" };
inline constexpr char kCryfsConfigFileName[] { "cryfs.config" };
inline constexpr char kCryfsType[] { "fuse.cryfs" };
inline constexpr char kVaultConfigFileName[] { "vaultConfig.ini" };
inline constexpr char kGroupPolicyKeyVaultAlgoName[] { "dfm.vault.algo.name" };
inline constexpr char kVaultTimeConfigFilePath[] { "/../dde-file-manager/vaultTimeConfig" };

inline constexpr char kConfigNodeName[] { "INFO" };
inline constexpr char kConfigKeyEncryptionMethod[] { "encryption_method" };
inline constexpr char kConfigValueMethodKey[] { "key_encryption" };
inline constexpr char kConfigValueMethodTransparent[] { "transparent_encryption" };
inline constexpr char kConfigValueMethodTpmWithoutPin[] { "tpmWithoutPin_encryption" };
inline constexpr char kConfigValueMethodTpmWithPin[] { "tpmWithPin_encryption" };
inline constexpr char kConfigKeyNotExist[] { "NoExist" };
inline constexpr char kConfigKeyAlgoName[] { "algoName" };

inline constexpr char kConfigNodeNameOfTPM[] { "TPM" };
inline constexpr char kConfigKeyPrimaryHashAlgo[] { "primary_hash_algo" };
inline constexpr char kConfigKeyPrimaryKeyAlgo[] { "primary_key_algo" };

enum TpmDecryptState {
    kDecryptSuccess = 0,
    kDecryptFailed,
    kNotAvailable
};

enum UnlockState {
    kUnlockSuccess = 0,
    kUnlockFailed,
    kUnlocking,
    kTpmNotAvailable
};

SERVERVAULT_END_NAMESPACE

#endif // SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
