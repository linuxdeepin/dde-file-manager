// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
                                              path.toStdString().c_str(), Q_NULLPTR);
    }
    return DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(), base.toStdString().c_str(),
                                          path.toStdString().c_str(), Q_NULLPTR);
}

QUrl VaultAssitControl::vaultUrlToLocalUrl(const QUrl &url)
{
    if (url.scheme() != scheme()) {
        fmWarning() << "No vault url, can't change to local url!";
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
