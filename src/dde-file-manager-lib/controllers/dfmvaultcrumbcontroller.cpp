// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvaultcrumbcontroller.h"
#include "controllers/pathmanager.h"
#include "singleton.h"
#include "controllers/vaultcontroller.h"
#include "dfileservices.h"

#include <QStorageInfo>

DFM_BEGIN_NAMESPACE

DFMVaultCrumbController::DFMVaultCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMVaultCrumbController::~DFMVaultCrumbController()
{

}

bool DFMVaultCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == DFMVAULT_SCHEME);
}

QList<CrumbData> DFMVaultCrumbController::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;
    QString prefixPath = "/";
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    const QString &path = info->toLocalFile();

    if (path.isEmpty()) {
        return list;
    }
    QStorageInfo storageInfo(path);
    QFile file(path);
    if (file.exists())
        prefixPath = storageInfo.rootPath() + "/";
    else
        prefixPath = VaultController::makeVaultLocalPath();
    // 设置地址栏保险箱图标
    QString text = Singleton<PathManager>::instance()->getSystemPathDisplayName("Vault");
    QString iconName = Singleton<PathManager>::instance()->getSystemPathIconName("Vault");
    CrumbData data(VaultController::makeVaultUrl(VaultController::makeVaultLocalPath()), text, iconName);
    list.append(data);

    DUrlList urlList = info->parentUrlList();
    urlList.insert(0, url);

    DAbstractFileInfoPointer infoPointer;
    // Push urls into crumb list (without prefix url)
    DUrlList::const_reverse_iterator iter = urlList.crbegin();
    while (iter != urlList.crend()) {
        const DUrl &oneUrl = *iter;

        QString localFile = oneUrl.toLocalFile();
        if (!prefixPath.startsWith(localFile)) {
            QString displayText = oneUrl.fileName();
            // Check for possible display text.
            infoPointer = DFileService::instance()->createFileInfo(nullptr, oneUrl);
            if (infoPointer) {
                displayText = infoPointer->fileDisplayName();
            }
            CrumbData crumbData(oneUrl, displayText);
            list.append(crumbData);
        }
        ++iter;
    }

    return list;
}

DFM_END_NAMESPACE
