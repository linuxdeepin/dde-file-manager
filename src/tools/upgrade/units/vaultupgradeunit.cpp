// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultupgradeunit.h"

#include <QLoggingCategory>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QProcess>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;

VaultUpgradeUnit::VaultUpgradeUnit()
    : UpgradeUnit()
{
    vaultBasePath = QDir::homePath() + QString("/.config/Vault");
    vaultBasePathOld = QDir::homePath() + QString("/.local/share/applications");

    cryfsConfigFilePathOld = vaultBasePathOld + QDir::separator() + QString(kVaultEncrypyDirName) + QDir::separator() + QString(kCryfsConfigFileName);
    cryfsConfigFilePathNew = vaultBasePath + QDir::separator() + QString(kVaultEncrypyDirName) + QDir::separator() + QString(kCryfsConfigFileName);
    decryptDirPathOld = vaultBasePathOld + QDir::separator() + QString(kVaultDecryptDirName);
    decryptDirPathNew = vaultBasePath + QDir::separator() + QString(kVaultDecryptDirName);
}

QString VaultUpgradeUnit::name()
{
    return "VaultUpgradeUnit";
}

bool VaultUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args)

    return (QFile::exists(cryfsConfigFilePathOld) && !QFile::exists(cryfsConfigFilePathNew));
}

bool VaultUpgradeUnit::upgrade()
{
    qCInfo(logToolUpgrade) << "Vault: start upgrade!";

    if (isLockState(decryptDirPathOld)) {
        qCWarning(logToolUpgrade) << "Vault: the old vault is unlock, so lock the old vault!";
        if (!lockVault(decryptDirPathOld)) {
            qCCritical(logToolUpgrade) << "Vault: lock vault failed, so upgrade vault failed!";
            return false;
        }
    }

    if (QFile::exists(vaultBasePath)) {
        qCCritical(logToolUpgrade) << "Vault: the new vault has exist, can't upgrade, you can remove the new vault, then restart system!";
        return false;
    }

    moveVault();

    qCInfo(logToolUpgrade) << "Vault: vault upgrade success!";
    return true;
}

void VaultUpgradeUnit::completed()
{
    qCInfo(logToolUpgrade) << "Vault: upgrade completed!";
}

void VaultUpgradeUnit::moveVault()
{
    QString vaultNewPath(vaultBasePath);
    QDir dir;
    if (!dir.exists(vaultNewPath)) {
        bool result = dir.mkpath(vaultNewPath);
        if (!result) {
            qCCritical(logToolUpgrade) << "Vault: create vault new path error when upgrade!";
            return;
        }
    }

    if (!dir.rename(decryptDirPathOld, decryptDirPathNew))
        qCWarning(logToolUpgrade) << "Vault: move decrypt dir failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << decryptDirPathOld << " to " << decryptDirPathNew;

    QString encryptDirPathOld = vaultBasePathOld + QDir::separator() + QString(kVaultEncrypyDirName);
    QString encryptDirPathNew = vaultBasePath + QDir::separator() + QString(kVaultEncrypyDirName);
    if (!dir.rename(encryptDirPathOld, encryptDirPathNew))
        qCWarning(logToolUpgrade) << "Vault: move encrypt dir failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << encryptDirPathOld << " to " << encryptDirPathNew;

    QString passwordFilePathOld = vaultBasePathOld + QDir::separator() + QString(kPasswordFileName);
    QString passwordFilePathNew = vaultBasePath + QDir::separator() + QString(kPasswordFileName);
    if (!QFile::rename(passwordFilePathOld, passwordFilePathNew))
        qCWarning(logToolUpgrade) << "Vault: move password file failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << passwordFilePathOld << " to " << passwordFilePathNew;

    QString rsaPubKeyFilePathOld = vaultBasePathOld + QDir::separator() + QString(kRSAPUBKeyFileName);
    QString rsaPubKeyFilePathNew = vaultBasePath + QDir::separator() + QString(kRSAPUBKeyFileName);
    if (!QFile::rename(rsaPubKeyFilePathOld, rsaPubKeyFilePathNew))
        qCWarning(logToolUpgrade) << "Vault: move pubkey file failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << rsaPubKeyFilePathOld << " to " << rsaPubKeyFilePathNew;

    QString rsaCipherFilePathOld = vaultBasePathOld + QDir::separator() + QString(kRSACiphertextFileName);
    QString rsaCipherFilePathNew = vaultBasePath + QDir::separator() + QString(kRSACiphertextFileName);
    if (!QFile::rename(rsaCipherFilePathOld, rsaCipherFilePathNew))
        qCWarning(logToolUpgrade) << "Vault: move cipher file failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << rsaCipherFilePathOld << " to " << rsaCipherFilePathNew;

    QString passwordHintFilePathOld = vaultBasePathOld + QDir::separator() + QString(kPasswordHintFileName);
    QString passwordHintFilePathNew = vaultBasePath + QDir::separator() + QString(kPasswordHintFileName);
    if (!QFile::rename(passwordHintFilePathOld, passwordHintFilePathNew))
        qCWarning(logToolUpgrade) << "Vault: move password hint file failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << passwordHintFilePathOld << " to " << passwordHintFilePathNew;

    QString configFilePathOld = vaultBasePathOld + QDir::separator() + QString(kVaultConfigFileName);
    QString configFilePahtNew = vaultBasePath + QDir::separator() + QString(kVaultConfigFileName);
    if (!QFile::rename(configFilePathOld, configFilePahtNew))
        qCWarning(logToolUpgrade) << "Vault: move config file failed!";
    else
        qCInfo(logToolUpgrade) << "Vault: mv " << configFilePathOld << " to " << configFilePahtNew;
}

bool VaultUpgradeUnit::isLockState(const QString &mountPath)
{
    if (!QFile::exists(mountPath))
        return false;

    QStorageInfo info(mountPath);
    if (info.isValid() && info.fileSystemType() == "fuse.cryfs") {
        return true;
    }

    return false;
}

bool VaultUpgradeUnit::lockVault(const QString &mountPath)
{
    QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
    QStringList arguments { "-zu", mountPath };

    if (fusermountBinary.isEmpty())
        return false;

    QProcess process;
    process.start(fusermountBinary, arguments);
    process.waitForStarted();
    process.waitForFinished();
    process.terminate();

    if (isLockState(mountPath))
        return false;

    return true;
}
