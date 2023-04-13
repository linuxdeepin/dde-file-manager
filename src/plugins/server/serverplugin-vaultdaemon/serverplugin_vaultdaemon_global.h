// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
#define SERVERPLUGIN_VAULTDAEMON_GLOBAL_H

#define SERVERVAULT_NAMESPCE serverplugin_vaultdaemon

#define SERVERVAULT_BEGIN_NAMESPACE namespace SERVERVAULT_NAMESPCE {
#define SERVERVAULT_END_NAMESPACE }
#define SERVERVAULT_USE_NAMESPACE using namespace SERVERVAULT_NAMESPCE;

#include <QString>
#include <QDir>

SERVERVAULT_BEGIN_NAMESPACE

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

SERVERVAULT_END_NAMESPACE

#endif // SERVERPLUGIN_VAULTDAEMON_GLOBAL_H
