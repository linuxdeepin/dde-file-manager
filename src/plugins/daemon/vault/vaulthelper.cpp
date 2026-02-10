// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulthelper.h"

#include <dfm-io/dfmio_utils.h>

#include <QUrl>

DAEMONPVAULT_USE_NAMESPACE

VaultHelper *VaultHelper::instance()
{
    static VaultHelper ins;
    return &ins;
}

bool VaultHelper::isVaultFile(const QUrl &url)
{
    bool isVault = false;
    if (url.scheme() == scheme()
            || url.path().startsWith(vaultMountDirLocalPath())) {
        isVault = true;
    }

    fmDebug() << "[VaultHelper::isVaultFile] URL:" << url.toString() << "is vault file:" << isVault;
    return isVault;
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
        fmDebug() << "[VaultHelper::vaultUrlToLocalUrl] URL not vault scheme, returning as-is:" << url.toString();
        return url;
    }

    QUrl localUrl;
    if (url.path().startsWith(vaultMountDirLocalPath())) {
        localUrl = QUrl::fromLocalFile(url.path());
    } else {
        localUrl = QUrl::fromLocalFile(vaultMountDirLocalPath() + url.path());
    }
    
    fmDebug() << "[VaultHelper::vaultUrlToLocalUrl] Converted vault URL:" << url.toString() << "to local URL:" << localUrl.toString();
    return localUrl;
}

QList<QUrl> VaultHelper::transUrlsToLocal(const QList<QUrl> &urls)
{
    QList<QUrl> urlsTrans {};

    int size = urls.size();
    fmDebug() << "[VaultHelper::transUrlsToLocal] Converting" << size << "URLs to local format";
    
    for (int i = 0; i < size; ++i) {
        const QUrl &url = urls.at(i);
        if (url.scheme() == scheme()) {
            urlsTrans.push_back(vaultUrlToLocalUrl(url));
        } else {
            urlsTrans.push_back(url);
        }
    }

    fmDebug() << "[VaultHelper::transUrlsToLocal] Converted" << urlsTrans.size() << "URLs successfully";
    return urlsTrans;
}

VaultHelper::VaultHelper(QObject *parent) : QObject(parent)
{
}
