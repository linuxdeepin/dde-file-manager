/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "stashmountsutils.h"
#include "utils/computerutils.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/configs/configsynchronizer.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/dfm_global_defines.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QUrl>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_computer {

namespace StashedMountsKeys {
const char *const kJsonGroup { "RemoteMounts" };
const char *const kHostKey { "host" };
const char *const kItemKey { "key" };
const char *const kNameKey { "name" };
const char *const kProtocolKey { "protocol" };
const char *const kShareKey { "share" };
}   // namespace StashedMountsKeys

static Settings *cfgSettings()
{
    return Application::genericSetting();
}

static constexpr char kStashedSmbDevices[] { "StashedSmbDevices" };
static constexpr char kSmbIntegrations[] { "SmbIntegrations" };

bool StashMountsUtils::isSmbIntegrationEnabled()
{
    DFMBASE_USE_NAMESPACE;
    return Application::genericAttribute(Application::kMergeTheEntriesOfSambaSharedFolders).toBool();
}

bool StashMountsUtils::isStashMountsEnabled()
{
    DFMBASE_USE_NAMESPACE;
    return Application::genericAttribute(Application::kAlwaysShowOfflineRemoteConnections).toBool();
}

QString StashMountsUtils::stashedConfigPath()
{
    DFMBASE_USE_NAMESPACE
    static QString path = StandardPaths::location(StandardPaths::kApplicationConfigPath) + "/deepin/dde-file-manager.json";
    return path;
}

QMap<QString, QString> StashMountsUtils::stashedMounts()
{
    QMap<QString, QString> ret;

    QStringList keys = cfgSettings()->keys(StashedMountsKeys::kJsonGroup).values();
    for (const auto &key : keys) {
        if (!key.startsWith(Global::Scheme::kSmb)) {   // old v1 version, which's key is start with '/run/user...'
            QMap<QString, QVariant> vals = cfgSettings()->value(StashedMountsKeys::kJsonGroup, key).toMap();
            if (vals.contains(StashedMountsKeys::kProtocolKey)
                && vals.contains(StashedMountsKeys::kHostKey)
                && vals.contains(StashedMountsKeys::kShareKey)
                && vals.contains(StashedMountsKeys::kNameKey)) {
                QString mountUrl = QString("%1://%2/%3/")
                                           .arg(vals.value(StashedMountsKeys::kProtocolKey).toString())
                                           .arg(vals.value(StashedMountsKeys::kHostKey).toString())
                                           .arg(vals.value(StashedMountsKeys::kShareKey).toString());
                QString displayName = vals.value(StashedMountsKeys::kNameKey).toString();

                ret.insert(mountUrl, displayName);
            }
        } else {
            ret.insert(key, cfgSettings()->value(StashedMountsKeys::kJsonGroup, key).toString());
        }
    }

    return ret;
}

QStringList StashMountsUtils::stashedSmbIntegrationUrls()
{
    return cfgSettings()->value(kStashedSmbDevices, kSmbIntegrations).toStringList();
}

QString StashMountsUtils::displayName(const QUrl &url)
{
    QString urlPath = ComputerUtils::getProtocolDevIdByStashedUrl(url);
    if (urlPath.isEmpty())
        return QObject::tr("Unknown");

    const auto &&mounts = stashedMounts();
    if (mounts.contains(urlPath))
        return mounts.value(urlPath);

    return QObject::tr("Unknown");
}

void StashMountsUtils::removeStashedMount(const QUrl &url)
{
    QString urlPath = ComputerUtils::getProtocolDevIdByStashedUrl(url);
    if (urlPath.isEmpty())
        return;

    QJsonDocument cfgDoc = cfgDocument();
    if (cfgDoc.isNull())
        return;

    // remove keyV2
    cfgSettings()->remove(StashedMountsKeys::kJsonGroup, urlPath);

    // remove v1 version
    QJsonObject obj = cfgDoc.object();
    QJsonValue mountsVal = obj.value(StashedMountsKeys::kJsonGroup);
    if (mountsVal.isObject()) {
        QJsonObject mountsObj = mountsVal.toObject();
        const QStringList &mountKeys = mountsObj.keys();
        QString cfgKeyV1;
        for (const auto &mountKey : mountKeys) {
            QJsonValue stashedData = mountsObj.value(mountKey);
            if (!stashedData.isObject())
                continue;

            QJsonObject stashedObj = stashedData.toObject();
            QString mountUrl = QString("%1://%2/%3/")
                                       .arg(stashedObj.value(StashedMountsKeys::kProtocolKey).toString())
                                       .arg(stashedObj.value(StashedMountsKeys::kHostKey).toString())
                                       .arg(stashedObj.value(StashedMountsKeys::kShareKey).toString());
            if (mountUrl == urlPath) {
                cfgKeyV1 = mountKey;
                qInfo() << mountKey << "is unstashed!";
                break;
            }
        }
        if (!cfgKeyV1.isEmpty()) {
            cfgSettings()->remove(StashedMountsKeys::kJsonGroup, cfgKeyV1);
        }
    }
}

void StashMountsUtils::stashMount(const QUrl &protocolUrl, const QString &displayName)
{
    if (!isStashMountsEnabled()) {
        qDebug() << "stash mounts is disabled";
        return;
    }

    const QString &id = ComputerUtils::getProtocolDevIdByUrl(protocolUrl);
    cfgSettings()->setValue(StashedMountsKeys::kJsonGroup, id, displayName);
}

void StashMountsUtils::stashSmbMount(const QVariantHash &newMount)
{
    if (!isStashMountsEnabled()) {
        qDebug() << "stash mounts is disabled";
        return;
    }

    const QString &key = QString("%1/smb-share:server=%2,share=%3")
                                 .arg(gvfsMountPath())
                                 .arg(newMount.value(StashedMountsKeys::kHostKey).toString())
                                 .arg(newMount.value(StashedMountsKeys::kShareKey).toString());

    cfgSettings()->setValue(StashedMountsKeys::kJsonGroup, key, newMount);
}

QUrl StashMountsUtils::makeStashedSmbMountUrl(const QVariantHash &stashedData)
{
    const QString &host = stashedData.value(StashedMountsKeys::kHostKey).toString();
    const QString &shareName = stashedData.value(StashedMountsKeys::kShareKey).toString();
    QUrl url;
    url.setScheme(Global::Scheme::kSmb);
    url.setHost(host);
    url.setPath("/" + shareName + "/");
    const QString &key = url.toString();
    const QUrl &stashedUrl = ComputerUtils::makeStashedProtocolDevUrl(key);

    return stashedUrl;
}

QVariantHash StashMountsUtils::makeStashedSmbDataById(const QString &id)
{
    QString displayName;
    QString host;
    QString shareName;
    QVariantHash newMount;
    if (DeviceUtils::isSamba(QUrl(id))) {
        const QUrl &url = QUrl::fromPercentEncoding(id.toUtf8());
        const QString &path = url.path();
        int pos = path.lastIndexOf("/");
        displayName = path.mid(pos + 1);
        host = displayName.section(" on ", 1, 1);
        shareName = displayName.section(" on ", 0, 0);
    } else if (id.startsWith(Global::Scheme::kSmb)) {
        host = QUrl(id).host();
        shareName = QUrl(id.endsWith("/") ? id.chopped(1) : id).fileName();
        displayName = QString("%1 on %2").arg(shareName).arg(host);
    }
    if (host.isEmpty() || shareName.isEmpty() || displayName.isEmpty())
        return newMount;

    newMount.insert(StashedMountsKeys::kHostKey, host);
    newMount.insert(StashedMountsKeys::kShareKey, shareName);
    newMount.insert(StashedMountsKeys::kProtocolKey, Global::Scheme::kSmb);
    newMount.insert(StashedMountsKeys::kNameKey, displayName);

    return newMount;
}

void StashMountsUtils::clearStashedMounts()
{
    cfgSettings()->removeGroup(StashedMountsKeys::kJsonGroup);
}

bool StashMountsUtils::isStashedDevExist(const QUrl &stashedUrl)
{
    QString id = ComputerUtils::getProtocolDevIdByStashedUrl(stashedUrl);
    return stashedMounts().contains(id);
}

void StashMountsUtils::stashMountedMounts()
{
    QStringList &&ids = DevProxyMng->getAllProtocolIds();
    for (auto id : ids) {
        QVariantHash newMount;
        if (id.startsWith(Global::Scheme::kSmb)) {
            DFMEntryFileInfoPointer info(new EntryFileInfo(ComputerUtils::makeProtocolDevUrl(id)));
            QString displayName = info->displayName();
            if (!displayName.isEmpty())
                cfgSettings()->setValue(StashedMountsKeys::kJsonGroup, id, displayName);

            newMount = makeStashedSmbDataById(id);
            if (!newMount.isEmpty())
                StashMountsUtils::stashSmbMount(newMount);
        } else if (DeviceUtils::isSamba(QUrl(id))) {
            newMount = makeStashedSmbDataById(id);
            if (!newMount.isEmpty())
                StashMountsUtils::stashSmbMount(newMount);
        }
    }
}

QString StashMountsUtils::gvfsMountPath()
{
    return SysInfoUtils::isRootUser()
            ? QString("/root/.gvfs")
            : QString("/run/user/%1/gvfs").arg(SysInfoUtils::getUserId());
}

void StashMountsUtils::bindStashEnableConf()
{
    static constexpr char kConfName[] { "org.deepin.dde.file-manager" };
    static constexpr char kKeyName[] { "dfm.samba.permanent" };
    SyncPair pair {
        { SettingType::kGenAttr, Application::kAlwaysShowOfflineRemoteConnections },
        { kConfName, kKeyName },
        saveStashEnableToConf,
        syncConfToAppSet
    };
    ConfigSynchronizer::instance()->watchChange(pair);
}

void StashMountsUtils::saveStashEnableToConf(const QVariant &var)
{
    DConfigManager::instance()->setValue(DConfigInfos::kConfName, DConfigInfos::kKeyName, var);
}

void StashMountsUtils::syncConfToAppSet(const QString &config, const QString &key, const QVariant &var)
{
    if (config == DConfigInfos::kConfName && key == DConfigInfos::kKeyName)
        Application::instance()->setGenericAttribute(Application::kAlwaysShowOfflineRemoteConnections, var.toBool());
}

QJsonDocument StashMountsUtils::cfgDocument()
{
    QFile cfgFile(stashedConfigPath());
    if (!cfgFile.open(QIODevice::ReadOnly)) {
        qWarning() << "cannnot open stashed config file: " << stashedConfigPath();
        return {};
    }

    QByteArray jsonData = cfgFile.readAll();
    cfgFile.close();

    QJsonParseError err;
    QJsonDocument cfgDoc = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "cannot parse config file! " << stashedConfigPath() << err.errorString();
        return {};
    }
    return cfgDoc;
}

}
