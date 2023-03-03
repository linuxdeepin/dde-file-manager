// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPER_GLOBAL_H
#define VAULTHELPER_GLOBAL_H

#include <QString>
#include <QDir>

namespace dfmplugin_utils {
inline const QString kVaultConfigPath(QDir::homePath() + QString("/.config/Vault"));
inline constexpr char kVaultMountDirName[] { "vault_unlocked" };
inline constexpr char kVaultBaseDirName[] { "vault_encrypted" };
inline constexpr char kCryfsConfigFileName[] { "cryfs.config" };
inline constexpr char kCryfsType[] { "fuse.cryfs" };
inline constexpr char kVaultConfigFileName[] { "vaultConfig.ini" };
inline constexpr char kGroupPolicyKeyVaultAlgoName[] { "dfm.vault.algo.name" };
inline constexpr char kVaultTimeConfigFilePath[] { "/../dde-file-manager/vaultTimeConfig" };
}

#ifdef COMPILE_ON_V23
#    define APP_SESSION_SERVICE "org.deepin.dde.SessionManager1"
#    define APP_SESSION_PATH "/org/deepin/dde/SessionManager1"
#else
#    define APP_SESSION_SERVICE "com.deepin.SessionManager"
#    define APP_SESSION_PATH "/com/deepin/SessionManager"
#endif

#endif   // VAULTHELPER_GLOBAL_H
