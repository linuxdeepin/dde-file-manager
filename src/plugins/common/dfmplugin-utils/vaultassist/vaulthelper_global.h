/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
}

#endif // VAULTHELPER_GLOBAL_H
