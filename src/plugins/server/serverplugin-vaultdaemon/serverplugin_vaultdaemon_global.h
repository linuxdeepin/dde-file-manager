// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
#define SERVERPLUGIN_VAULTDAEMON_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define SERVERVAULT_NAMESPCE serverplugin_vaultdaemon

#define SERVERVAULT_BEGIN_NAMESPACE namespace SERVERVAULT_NAMESPCE {
#define SERVERVAULT_END_NAMESPACE }
#define SERVERVAULT_USE_NAMESPACE using namespace SERVERVAULT_NAMESPCE;

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
inline constexpr char kVaultTimeConfigFileName[] { "vaultTimeConfig.json" };

inline constexpr char kNetWorkDBusServiceName[] { "org.deepin.service.SystemNetwork" };
inline constexpr char kNetWorkDBusPath[] { "/org/deepin/service/SystemNetwork" };
inline constexpr char kNetWorkDBusInterfaces[] { "org.deepin.service.SystemNetwork" };

enum class Connectivity {
    Unknownconnectivity = 0,
    Noconnectivity,
    Portal,
    Limited,
    Full        // 主机已连接到网络，并且似乎能够访问完整的Internet
};

enum VaultState {
    kUnknow = 0,
    kNotExisted,
    kEncrypted,
    kUnlocked,
    kUnderProcess,
    kBroken,
    kNotAvailable
};

inline constexpr char kVaultDConfigName[] { "org.deepin.dde.file-manager.vault" };

SERVERVAULT_END_NAMESPACE

#endif   // SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
