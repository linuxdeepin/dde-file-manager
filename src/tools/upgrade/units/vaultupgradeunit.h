// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTUPGRADEUNIT_H
#define VAULTUPGRADEUNIT_H

#include "core/upgradeunit.h"

#include <QDir>

namespace dfm_upgrade {

inline constexpr char kVaultDecryptDirName[] { "vault_unlocked" };
inline constexpr char kVaultEncrypyDirName[] { "vault_encrypted" };
inline constexpr char kPasswordFileName[] { "pbkdf2clipher" };
inline constexpr char kRSAPUBKeyFileName[] { "rsapubkey" };
inline constexpr char kRSACiphertextFileName[] { "rsaclipher" };
inline constexpr char kPasswordHintFileName[] { "passwordHint" };
inline constexpr char kVaultConfigFileName[] { "vaultConfig.ini" };
inline constexpr char kCryfsConfigFileName[] { "cryfs.config" };

class VaultUpgradeUnit : public UpgradeUnit
{
public:
    VaultUpgradeUnit();

    QString name() override;
    bool initialize(const QMap<QString, QString> &args) override;
    bool upgrade() override;
    void completed() override;

private:
    void moveVault();
    bool isLockState(const QString &mountPath);
    bool lockVault(const QString &mountPath);

    QString cryfsConfigFilePathOld;
    QString cryfsConfigFilePathNew;
    QString decryptDirPathOld;
    QString decryptDirPathNew;

    QString vaultBasePath;
    QString vaultBasePathOld;
};
}

#endif // VAULTUPGRADEUNIT_H
