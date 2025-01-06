// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulthelper.h"

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDebug>

DAEMONPVAULT_USE_NAMESPACE

VaultHelper *VaultHelper::instance()
{
    static VaultHelper ins;
    return &ins;
}

bool VaultHelper::isVaultFile(const QUrl &url)
{
    if (url.scheme() == scheme()
            || url.path().startsWith(vaultMountDirLocalPath())) {
        return true;
    }

    return false;
}

QString VaultHelper::vaultBaseDirLocalPath()
{
    return buildVaultLocalPath("", kVaultBaseDirName);
}

QString VaultHelper::vaultMountDirLocalPath()
{
    return buildVaultLocalPath("", kVaultMountDirName);
}

QString VaultHelper::buildVaultLocalPath(const QString &path, const QString &base)
{
    if (base.isEmpty()) {
        return DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(), QString(kVaultMountDirName).toStdString().c_str(),
                                              path.toStdString().c_str(), nullptr);
    }
    return DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(), base.toStdString().c_str(),
                                          path.toStdString().c_str(), nullptr);
}

QUrl VaultHelper::vaultUrlToLocalUrl(const QUrl &url)
{
    if (url.scheme() != scheme()) {
        return url;
    }

    if (url.path().startsWith(vaultMountDirLocalPath())) {
        return QUrl::fromLocalFile(url.path());
    } else {
        return QUrl::fromLocalFile(vaultMountDirLocalPath() + url.path());
    }
}

QList<QUrl> VaultHelper::transUrlsToLocal(const QList<QUrl> &urls)
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

VaultHelper::VaultHelper(QObject *parent) : QObject(parent)
{
}
