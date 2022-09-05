// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmfilecrumbcontroller.h"
#include "controllers/pathmanager.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "deviceinfo/udisklistener.h"

#include "dfileservices.h"
#include "dfileinfo.h"
#include "app/define.h"
#include "singleton.h"

#include <QStandardPaths>
#include <QStorageInfo>

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMFileCrumbController::DFMFileCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
    , homePath(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last())
{

}

DFMFileCrumbController::~DFMFileCrumbController()
{

}

bool DFMFileCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == FILE_SCHEME);
}

QList<CrumbData> DFMFileCrumbController::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;
    QString prefixPath = "/";
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    const QString &path = info->toLocalFile();

    if (path.isEmpty()) {
        return list;
    }

    if (path.startsWith(homePath)) {
        prefixPath = homePath;
        QString iconName = Singleton<PathManager>::instance()->getSystemPathIconName("Home");
        CrumbData data(DUrl::fromLocalFile(homePath), getDisplayName("Home"), iconName);
        list.append(data);
    } else {
        QStorageInfo storageInfo(path);
        UDiskDeviceInfoPointer deviceInfo = deviceListener->getDeviceByPath(path);
        if (!deviceInfo) {
            deviceInfo = deviceListener->getDeviceByFilePath(path);
        }
        QString iconName = QStringLiteral("drive-harddisk-symbolic");
        prefixPath = storageInfo.rootPath();
        if (deviceInfo) {
            switch (deviceInfo->getMediaType()) {
            case UDiskDeviceInfo::MediaType::removable:
                iconName = QStringLiteral("drive-removable-media-symbolic");
                break;
            case UDiskDeviceInfo::MediaType::dvd:
                iconName = QStringLiteral("media-optical-symbolic");
                break;
            case UDiskDeviceInfo::MediaType::phone:
                iconName = QStringLiteral("phone-symbolic");
                break;
            case UDiskDeviceInfo::MediaType::iphone:
                iconName = QStringLiteral("phone-apple-iphone-symbolic");
                break;
            case UDiskDeviceInfo::MediaType::camera:
                iconName = QStringLiteral("camera-photo-symbolic");
                break;
            default:
                break;
            }
        }

        if (prefixPath == "/") {
            CrumbData data(DUrl(FILE_ROOT), getDisplayName("System Disk"), "drive-harddisk-root-symbolic");
            list.append(data);
        } else {
            CrumbData data(DUrl::fromLocalFile(prefixPath), QString(), iconName);
            list.append(data);
        }
    }

    DUrlList urlList = info->parentUrlList();
    urlList.insert(0, url);

    DAbstractFileInfoPointer infoPointer;
    // Push urls into crumb list (without prefix url)
    DUrlList::const_reverse_iterator iter = urlList.crbegin();
    while (iter != urlList.crend()) {
        const DUrl &oneUrl = *iter;
        QString localFile = oneUrl.toLocalFile();
        if (!prefixPath.startsWith(oneUrl.toLocalFile())) {
            QString displayText = oneUrl.fileName();
            // Check for possible display text.
            infoPointer = DFileService::instance()->createFileInfo(nullptr, oneUrl);
            if (infoPointer) {
                displayText = infoPointer->fileDisplayName();
            }
            CrumbData data(oneUrl, displayText);
            list.append(data);
        }
        ++iter;
    }

    return list;
}

// blumia: avoid using this, recommand using the following way:
//         DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, oneUrl);
//         then you can get a proper display name via `info->fileDisplayName()`.
//         We are using this since not all path can got a proper display name yet.
// TODO: When backend is ready, switch to DAbstractFileInfo::fileDisplayName().
QString DFMFileCrumbController::getDisplayName(const QString &name) const
{
    QString text = Singleton<PathManager>::instance()->getSystemPathDisplayName(name);
    return text.isEmpty() ? name : text;
}

DFM_END_NAMESPACE
