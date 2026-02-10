// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradeutils.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QLoggingCategory>
#include <QDir>
#include <QDateTime>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

namespace dfm_upgrade {
namespace UpgradeUtils {
QVariant genericAttribute(const QString &key)
{
    static constexpr char kGenAttrKey[] { "GenericAttribute" };
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.count() > 0) {
        auto genCfgPath = paths.first() + "/deepin/dde-file-manager.json";
        qCInfo(logToolUpgrade) << "upgrade: genericAttribute config path: " << genCfgPath;

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

void addOldGenericAttribute(const QJsonArray &array)
{
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.isEmpty()) {
        qCWarning(logToolUpgrade) << "No config location paths available for adding old generic attributes";
        return;
    }

    auto genCfgPath = paths.first() + "/deepin/dde-file-manager.json";
    QFile cfgFile(genCfgPath);

    if (!cfgFile.open(QIODevice::ReadOnly)) {
        qCWarning(logToolUpgrade) << "Failed to open config file for reading during old attribute addition:" << genCfgPath;
        return;
    }

    auto datas = cfgFile.readAll();
    cfgFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(datas);
    if (!doc.isObject())
        return;

    qCInfo(logToolUpgrade) << "upgrade: addOldGenericAttribute config path: " << genCfgPath;
    auto rootObj = doc.object();
    auto genObj = rootObj.value("GenericAttribute").toObject();
    genObj.insert("OldAttributes", array);
    rootObj["GenericAttribute"] = genObj;
    QJsonDocument newDoc(rootObj);
    QByteArray data = newDoc.toJson();

    QFile file(genCfgPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCCritical(logToolUpgrade) << "Failed to open config file for writing old attributes:" << genCfgPath;
        return;
    }

    file.write(data);
    file.close();
}

QVariant applicationAttribute(const QString &key)
{
    static constexpr char kAppAttrKey[] { "ApplicationAttribute" };
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.count() > 0) {
        auto appCfgPath = paths.first() + "/deepin/dde-file-manager/dde-file-manager.json";
        qCInfo(logToolUpgrade) << "upgrade: applicationAttribute config path: " << appCfgPath;

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

bool backupFile(const QString &sourceFile, const QString &backupDirPath)
{
    // 创建备份目录
    QDir backupDir(backupDirPath);
    if (!backupDir.exists()) {
        if (!backupDir.mkpath(".")) {
            qCCritical(logToolUpgrade) << "Failed to create backup directory:" << backupDirPath;
            return false;
        }
    }

    // 获取源文件名
    QFileInfo sourceFileInfo(sourceFile);
    QString sourceFileName { sourceFileInfo.fileName() };
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    // 构建备份文件路径
    QString backupFilePath { backupDirPath + "/" + sourceFileName + "." + timestamp };

    // 备份文件
    if (!QFile::copy(sourceFile, backupFilePath)) {
        qCCritical(logToolUpgrade) << "Failed to copy file from" << sourceFile << "to" << backupFilePath;
        return false;
    }

    return true;
}

}   // namespace UpgradeUtils
}   // namespace dfm_upgrade
