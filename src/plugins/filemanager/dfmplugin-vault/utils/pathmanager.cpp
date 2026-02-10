// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathmanager.h"
#include "vaultdefine.h"

#include <dfm-io/dfmio_utils.h>

using namespace dfmplugin_vault;

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
