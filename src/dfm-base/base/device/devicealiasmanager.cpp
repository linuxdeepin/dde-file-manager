// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicealiasmanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/networkutils.h>

#include <QUrl>
#include <QRegularExpression>

using namespace dfmbase;
DFMBASE_USE_NAMESPACE

inline constexpr char kCfgName[] { "org.deepin.dde.file-manager.mount" };
inline constexpr char kNPDAlias[] { "aliasSupportedProtocolList" };

inline constexpr char kSchemeSupportedList[] { "hideMyDirectories" };
inline constexpr char kNPAliasGroupName[] { "NetworkProtocolDeviceAlias" };
inline constexpr char kAliasItemName[] { "Items" };
inline constexpr char kSchemeKey[] { "scheme" };
inline constexpr char kDevicesKey[] { "devices" };
inline constexpr char kHostKey[] { "host" };
inline constexpr char kAliasKey[] { "alias" };

// NPDeviceAliasManager implementation
NPDeviceAliasManager::NPDeviceAliasManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(logDFMBase) << "EntryEntityAliasManager initialized";
}

QUrl NPDeviceAliasManager::convertToProtocolUrl(const QUrl &url) const
{
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "Invalid URL provided:" << url.toString();
        return url;
    }

    if (url.scheme() != Global::Scheme::kFile)
        return url;

    static QRegularExpression regex(R"(host=([0-9]{1,3}(?:\.[0-9]{1,3}){3}))");
    auto match = regex.match(url.path());
    if (!match.hasMatch())
        return url;

    QUrl protocolUrl;
    auto host = match.captured(1);
    protocolUrl.setHost(host);
    if (ProtocolUtils::isNFSFile(url))
        protocolUrl.setScheme(Global::Scheme::kNfs);
    else if (ProtocolUtils::isSMBFile(url))
        protocolUrl.setScheme(Global::Scheme::kSmb);
    else if (ProtocolUtils::isSFTPFile(url))
        protocolUrl.setScheme(Global::Scheme::kSFtp);
    else if (ProtocolUtils::isFTPFile(url))
        protocolUrl.setScheme(Global::Scheme::kFtp);
    else if (ProtocolUtils::isDavFile(url))
        protocolUrl.setScheme(Global::Scheme::kDav);
    else if (ProtocolUtils::isDavsFile(url))
        protocolUrl.setScheme(Global::Scheme::kDavs);

    return protocolUrl.isValid() ? protocolUrl : url;
}

NPDeviceAliasManager *NPDeviceAliasManager::instance()
{
    static NPDeviceAliasManager ins;
    return &ins;
}

QString NPDeviceAliasManager::getAlias(const QUrl &protocolUrl) const
{
    const auto &url = convertToProtocolUrl(protocolUrl);
    if (!canSetAlias(url))
        return "";

    QReadLocker lk(&rwLock);
    const auto &itemList = Application::genericSetting()->value(kNPAliasGroupName, kAliasItemName).toList();
    for (const auto &item : std::as_const(itemList)) {
        const auto &map = item.toMap();
        const auto &scheme = map.value(kSchemeKey).toString();
        if (scheme != url.scheme())
            continue;

        const auto &deviceList = map.value(kDevicesKey).toList();
        for (const auto &dev : std::as_const(deviceList)) {
            const auto &devMap = dev.toMap();
            const auto &host = devMap.value(kHostKey).toString();
            if (host == url.host())
                return devMap.value(kAliasKey).toString();
        }
    }

    return "";
}

/**
 * @brief Set or remove alias for a protocol device.
 * @param protocolUrl QUrl, the protocol device url (e.g. smb://10.10.10.10)
 * @param alias QString, the alias to set. If empty, remove the alias entry.
 * @return true if set/remove succeeded, false otherwise.
 */
bool NPDeviceAliasManager::setAlias(const QUrl &protocolUrl, const QString &alias)
{
    const auto &url = convertToProtocolUrl(protocolUrl);
    if (!canSetAlias(url))
        return false;

    QReadLocker rlk(&rwLock);
    auto itemList = Application::genericSetting()->value(kNPAliasGroupName, kAliasItemName).toList();
    rlk.unlock();
    bool modified = false;

    // Iterate over protocol schemes
    for (int i = 0; i < itemList.size(); ++i) {
        QVariantMap map = itemList[i].toMap();
        const QString scheme = map.value(kSchemeKey).toString();
        if (scheme != url.scheme())
            continue;

        QList<QVariant> deviceList = map.value(kDevicesKey).toList();
        bool found = false;

        // Search for the device by host
        for (int j = 0; j < deviceList.size(); ++j) {
            QVariantMap devMap = deviceList[j].toMap();
            const QString host = devMap.value(kHostKey).toString();
            const QString &oldAlias = devMap.value(kAliasKey).toString();
            if (host == url.host()) {
                found = true;
                if (alias.isEmpty() || alias == host) {
                    // Remove the device entry if alias is empty
                    deviceList.removeAt(j);
                    qCInfo(logDFMBase) << "Alias removed for" << url.toString();
                } else if (alias == oldAlias) {
                    return true;
                } else {
                    // Set or update the alias
                    devMap[kAliasKey] = alias;
                    deviceList[j] = devMap;
                    qCInfo(logDFMBase) << "Alias set for" << url.toString() << ":" << alias;
                }
                modified = true;
                break;
            }
        }

        // If not found and alias is not empty, add new device entry
        if (!found && !alias.isEmpty()) {
            QVariantMap newDev;
            newDev[kHostKey] = url.host();
            newDev[kAliasKey] = alias;
            deviceList.append(newDev);
            qCInfo(logDFMBase) << "Alias added for" << url.toString() << ":" << alias;
            modified = true;
        }

        // Update the device list in the map
        map[kDevicesKey] = deviceList;
        itemList[i] = map;
        // Only one scheme group should match, so break
        break;
    }

    // If scheme group not found and alias is not empty, add new group
    if (!modified && !alias.isEmpty()) {
        QVariantMap newMap;
        newMap[kSchemeKey] = url.scheme();
        QVariantMap newDev;
        newDev[kHostKey] = url.host();
        newDev[kAliasKey] = alias;
        newMap[kDevicesKey] = QVariantList { newDev };
        itemList.append(newMap);
        qCInfo(logDFMBase) << "Alias group and device added for" << url.toString() << ":" << alias;
        modified = true;
    }

    if (modified) {
        QWriteLocker lk(&rwLock);
        Application::genericSetting()->setValue(kNPAliasGroupName, kAliasItemName, itemList);
    }

    return true;
}

void NPDeviceAliasManager::removeAlias(const QUrl &protocolUrl)
{
    setAlias(protocolUrl, QString());
}

bool NPDeviceAliasManager::hasAlias(const QUrl &protocolUrl) const
{
    return !getAlias(protocolUrl).isEmpty();
}

bool NPDeviceAliasManager::canSetAlias(const QUrl &protocolUrl) const
{
    const auto &url = convertToProtocolUrl(protocolUrl);
    if (!url.isValid())
        return false;

    QReadLocker lk(&rwLock);
    const auto &list = DConfigManager::instance()->value(kCfgName, kNPDAlias).toStringList();
    return list.contains(url.scheme(), Qt::CaseInsensitive);
}
