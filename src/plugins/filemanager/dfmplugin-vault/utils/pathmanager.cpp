/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

QString PathManager::makeVaultLocalPath(QString path, QString base)
{
    if (base.isEmpty()) {
        base = kVaultDecryptDirName;
    }
    return DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), base.toStdString().c_str(),
                                          path.toStdString().c_str(), nullptr);
}
