/*
* Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
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
#include "vaultassitcontrol.h"
#include "vaulthelper_global.h"

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>

DPUTILS_USE_NAMESPACE

VaultAssitControl::VaultAssitControl(QObject *parent) : QObject(parent)
{

}

VaultAssitControl *VaultAssitControl::instance()
{
    static VaultAssitControl ins;
    return &ins;
}

bool VaultAssitControl::isVaultFile(const QUrl &url)
{
    if (url.scheme() == scheme()
            || url.path().startsWith(vaultMountDirLocalPath())) {
        return true;
    }

    return false;
}

QString VaultAssitControl::vaultBaseDirLocalPath()
{
    return buildVaultLocalPath("", kVaultBaseDirName);
}

QString VaultAssitControl::vaultMountDirLocalPath()
{
    return buildVaultLocalPath("", kVaultMountDirName);
}

QString VaultAssitControl::buildVaultLocalPath(const QString &path, const QString &base)
{
    if (base.isEmpty()) {
        return DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(), QString(kVaultMountDirName).toStdString().c_str(),
                                              path.toStdString().c_str(), nullptr);
    }
    return DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(), base.toStdString().c_str(),
                                          path.toStdString().c_str(), nullptr);
}

QUrl VaultAssitControl::vaultUrlToLocalUrl(const QUrl &url)
{
    if (url.scheme() != scheme()) {
        qWarning() << "No vault url, can't change to local url!";
        return url;
    }

    if (url.path().startsWith(vaultMountDirLocalPath())) {
        return QUrl::fromLocalFile(url.path());
    } else {
        return QUrl::fromLocalFile(vaultMountDirLocalPath() + url.path());
    }
}

QList<QUrl> VaultAssitControl::transUrlsToLocal(const QList<QUrl> &urls)
{
    QList<QUrl> urlsTrans {};

    int size = urls.size();
    for (int i = 0; i < size; ++i) {
        const QUrl &url = urls.at(i);
        if (url.scheme() == scheme()) {
            urlsTrans.push_back(vaultUrlToLocalUrl(url));
        } else {
            urlsTrans.push_back(url);
        }
    }

    return urlsTrans;
}
