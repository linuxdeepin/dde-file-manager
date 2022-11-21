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

class StashMountsUtils
{
public:
    static bool isStashMountsEnabled();
    static QString stashedConfigPath();

    static QMap<QString, QString> stashedMounts();
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

private:
    static QJsonDocument cfgDocument();
};

}

#endif   // STASHMOUNTSMANAGER_H
