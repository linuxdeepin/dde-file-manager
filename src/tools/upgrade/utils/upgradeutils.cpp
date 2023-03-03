// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradeutils.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

namespace dfm_upgrade {
namespace UpgradeUtils {
QVariant genericAttribute(const QString &key)
{
    static constexpr char kGenAttrKey[] { "GenericAttribute" };
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.count() > 0) {
        auto genCfgPath = paths.first() + "/deepin/dde-file-manager.json";
        qInfo() << "upgrade: genericAttribute config path: " << genCfgPath;

        QFile cfgFile(genCfgPath);
        if (cfgFile.open(QIODevice::ReadOnly)) {
            auto datas = cfgFile.readAll();
            cfgFile.close();

            QJsonDocument doc = QJsonDocument::fromJson(datas);
            if (doc.isObject()) {
                auto rootObj = doc.object();
                if (rootObj.contains(kGenAttrKey) && rootObj.value(kGenAttrKey).isObject()) {
                    auto genAttr = rootObj.value(kGenAttrKey).toObject();
                    if (genAttr.contains(key))
                        return genAttr.value(key).toVariant();
                }
            }
        }
    }
    return QVariant();
}

QVariant applicationAttribute(const QString &key)
{
    static constexpr char kAppAttrKey[] { "ApplicationAttribute" };
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.count() > 0) {
        auto appCfgPath = paths.first() + "/deepin/dde-file-manager/dde-file-manager.json";
        qInfo() << "upgrade: applicationAttribute config path: " << appCfgPath;

        QFile cfgFile(appCfgPath);
        if (cfgFile.open(QIODevice::ReadOnly)) {
            auto datas = cfgFile.readAll();
            cfgFile.close();

            QJsonDocument doc = QJsonDocument::fromJson(datas);
            if (doc.isObject()) {
                auto rootObj = doc.object();
                if (rootObj.contains(kAppAttrKey) && rootObj.value(kAppAttrKey).isObject()) {
                    auto appAttr = rootObj.value(kAppAttrKey).toObject();
                    if (appAttr.contains(key))
                        return appAttr.value(key).toVariant();
                }
            }
        }
    }
    return QVariant();
}

}   // namespace UpgradeUtils
}   // namespace dfm_upgrade
