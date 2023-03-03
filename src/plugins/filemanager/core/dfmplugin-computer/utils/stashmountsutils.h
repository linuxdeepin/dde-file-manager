// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STASHMOUNTSMANAGER_H
#define STASHMOUNTSMANAGER_H

#include "dfmplugin_computer_global.h"

#include <QVariantMap>
#include <QJsonDocument>

namespace dfmplugin_computer {

namespace StashedMountsKeys {
extern const char *const kJsonGroup;
extern const char *const kHostKey;
extern const char *const kItemKey;
extern const char *const kNameKey;
extern const char *const kProtocolKey;
extern const char *const kShareKey;
}   // namespace StashedMountsKeys

namespace DConfigInfos {
inline constexpr char kConfName[] { "org.deepin.dde.file-manager" };
inline constexpr char kKeyName[] { "dfm.samba.permanent" };
}   // namespace DConfigInfos

class StashMountsUtils
{
public:
    static bool isSmbIntegrationEnabled();
    static bool isStashMountsEnabled();
    static QString stashedConfigPath();

    static QMap<QString, QString> stashedMounts();
    static QStringList stashedSmbIntegrationUrls();
    static QString displayName(const QUrl &url);
    static void removeStashedMount(const QUrl &url);
    static void stashMount(const QUrl &protocolUrl, const QString &displayName);
    static QVariantHash makeStashedSmbDataById(const QString &id);
    static void stashSmbMount(const QVariantHash &newMount);   // for compatible issue
    static QUrl makeStashedSmbMountUrl(const QVariantHash &stashedData);   // for compatible issue
    static void clearStashedMounts();
    static bool isStashedDevExist(const QUrl &stashedUrl);
    static void stashMountedMounts();
    static QString gvfsMountPath();

    static void bindStashEnableConf();
    static void saveStashEnableToConf(const QVariant &var);
    static void syncConfToAppSet(const QString &, const QString &, const QVariant &);

private:
    static QJsonDocument cfgDocument();
};

}

#endif   // STASHMOUNTSMANAGER_H
