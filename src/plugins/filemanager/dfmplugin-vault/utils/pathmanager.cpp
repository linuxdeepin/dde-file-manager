// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathmanager.h"
#include "vaultdefine.h"

#include <dfm-base/utils/dialogmanager.h>

#include <dfm-io/dfmio_utils.h>

using namespace dfmplugin_vault;
DFMBASE_USE_NAMESPACE

PathManager::PathManager(QObject *parent)
    : QObject(parent)
{
}

QString PathManager::vaultLockPath()
{
    return makeVaultLocalPath("", kVaultEncrypyDirName);
}

QString PathManager::vaultUnlockPath()
{
    return makeVaultLocalPath("", kVaultDecryptDirName);
}

QString PathManager::vaultPswContainerPath(const QString &baseDirPath)
{
    return baseDirPath + QDir::separator() + kVaultPswContainerFileName;
}

QString PathManager::vaultEncryptPath(const QString &baseDirPath)
{
    return baseDirPath + QDir::separator() + kVaultEncrypyDirName;
}

QString PathManager::vaultMountPath(const QString &baseDirPath)
{
    return baseDirPath + QDir::separator() + kVaultDecryptDirName;
}

QString PathManager::makeVaultLocalPath(const QString &path, const QString &base)
{
    if (base.isEmpty()) {
        return DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), QString(kVaultDecryptDirName).toStdString().c_str(),
                                              path.toStdString().c_str(), nullptr);
    }
    return DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), base.toStdString().c_str(),
                                          path.toStdString().c_str(), nullptr);
}

QString PathManager::addPathSlash(const QString &path)
{
    return DFMIO::DFMUtils::buildFilePath(path.toStdString().c_str(),
                                          QString("/").toStdString().c_str(), nullptr);
}

bool PathManager::createDirIfNotExist(const QString &path)
{
    if (!QFile::exists(path)) {
        QDir().mkpath(path);
    } else {
        QDir dir(path);
        if (!dir.isEmpty()) {
            fmCritical() << "Vault: Create vault dir failed, dir is not empty!";
            return false;
        }
    }
    return true;
}

bool PathManager::createVaultMountDir(const QString &vaultBasePath)
{
    QString mountDir = PathManager::vaultMountPath(vaultBasePath);
    if (!createDirIfNotExist(mountDir)) {
        DialogManager::instance()->showErrorDialog(tr("Unlock failed"),
                                                   tr("The %1 directory is occupied,\n "
                                                      "please clear the files in this directory and try to unlock the safe again.")
                                                           .arg(mountDir));
        return false;
    }
    return true;
}
