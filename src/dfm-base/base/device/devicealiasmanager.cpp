// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicealiasmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QUrl>

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

NPDeviceAliasManager *NPDeviceAliasManager::instance()
{
    static NPDeviceAliasManager ins;
    return &ins;
}

QString NPDeviceAliasManager::getAlias(const QUrl &protocolUrl) const
{
    if (!canSetAlias(protocolUrl))
        return "";

    const auto &itemList = Application::genericSetting()->value(kNPAliasGroupName, kAliasItemName).toList();
    for (const auto &item : std::as_const(itemList)) {
        const auto &map = item.toMap();
        const auto &scheme = map.value(kSchemeKey).toString();
        if (scheme != protocolUrl.scheme())
            continue;

        const auto &deviceList = map.value(kDevicesKey).toList();
        for (const auto &dev : std::as_const(deviceList)) {
            const auto &devMap = dev.toMap();
            const auto &host = devMap.value(kHostKey).toString();
            if (host == protocolUrl.host())
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
    if (!canSetAlias(protocolUrl))
        return false;

    auto itemList = Application::genericSetting()->value(kNPAliasGroupName, kAliasItemName).toList();
    bool modified = false;

    // Iterate over protocol schemes
    for (int i = 0; i < itemList.size(); ++i) {
        QVariantMap map = itemList[i].toMap();
        const QString scheme = map.value(kSchemeKey).toString();
        if (scheme != protocolUrl.scheme())
            continue;

        QList<QVariant> deviceList = map.value(kDevicesKey).toList();
        bool found = false;

        // Search for the device by host
        for (int j = 0; j < deviceList.size(); ++j) {
            QVariantMap devMap = deviceList[j].toMap();
            const QString host = devMap.value(kHostKey).toString();
            const QString &oldAlias = devMap.value(kAliasKey).toString();
            if (host == protocolUrl.host()) {
                found = true;
                if (alias.isEmpty() || alias == host) {
                    // Remove the device entry if alias is empty
                    deviceList.removeAt(j);
                    qCInfo(logDFMBase) << "Alias removed for" << protocolUrl.toString();
                } else if (alias == oldAlias) {
                    return true;
                } else {
                    // Set or update the alias
                    devMap[kAliasKey] = alias;
                    deviceList[j] = devMap;
                    qCInfo(logDFMBase) << "Alias set for" << protocolUrl.toString() << ":" << alias;
                }
                modified = true;
                break;
            }
        }

        // If not found and alias is not empty, add new device entry
        if (!found && !alias.isEmpty()) {
            QVariantMap newDev;
            newDev[kHostKey] = protocolUrl.host();
            newDev[kAliasKey] = alias;
            deviceList.append(newDev);
            qCInfo(logDFMBase) << "Alias added for" << protocolUrl.toString() << ":" << alias;
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
        newMap[kSchemeKey] = protocolUrl.scheme();
        QVariantMap newDev;
        newDev[kHostKey] = protocolUrl.host();
        newDev[kAliasKey] = alias;
        newMap[kDevicesKey] = QVariantList { newDev };
        itemList.append(newMap);
        qCInfo(logDFMBase) << "Alias group and device added for" << protocolUrl.toString() << ":" << alias;
        modified = true;
    }

    if (modified)
        Application::genericSetting()->setValue(kNPAliasGroupName, kAliasItemName, itemList);

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
    if (!protocolUrl.isValid() || protocolUrl.host().isEmpty())
        return false;

    const auto &list = DConfigManager::instance()->value(kCfgName, kNPDAlias).toStringList();
    return list.contains(protocolUrl.scheme(), Qt::CaseInsensitive);
}
