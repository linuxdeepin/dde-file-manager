// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appattributeupgradeunit.h"

#include <dfm-base/dfm_log_defines.h>

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;

static constexpr char kConfigGroupAppAttribute[] { "ApplicationAttribute" };
static constexpr char kConfigKeyIconSizeLevel[] { "IconSizeLevel" };
static constexpr char kAppAttributeVersion[] { "v1.0" };
static constexpr int kOldMaxIconSizeLevel { 4 };

static QString kConfigurationPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager/dde-file-manager.json";
static QString kBackupDirPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager/old";

AppAttributeUpgradeUnit::AppAttributeUpgradeUnit()
    : UpgradeUnit()
{
}

QString AppAttributeUpgradeUnit::name()
{
    return "AppAttributeUpgradeUnit";
}

bool AppAttributeUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args)

    QFile file(kConfigurationPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    if (!backupAppAttribute())
        return false;

    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    configObject = doc.object();

    if (!configObject.contains(kConfigGroupAppAttribute)) {
        qCWarning(logToolUpgrade) << "upgrade: application attribute not found.";
        return false;
    }

    auto appAttribute = configObject.value(kConfigGroupAppAttribute).toObject();
    if (!appAttribute.keys().contains(kConfigKeyIconSizeLevel)) {
        qCWarning(logToolUpgrade) << "upgrade: iconSizeLevel not found.";
        return false;
    }

    oldIconSizeLevel = appAttribute.value(kConfigKeyIconSizeLevel).toInt();

    if (oldIconSizeLevel < 0) {
        qCWarning(logToolUpgrade) << "upgrade: iconSizeLevel is invalid: " << oldIconSizeLevel;
        return false;
    }

    if (oldIconSizeLevel > kOldMaxIconSizeLevel) {
        qCWarning(logToolUpgrade) << "upgrade: iconSizeLevel is already have new value.";
        return false;
    }

    return true;
}

bool AppAttributeUpgradeUnit::upgrade()
{
    int newIconSizeLevel = transIconSizeLevel(oldIconSizeLevel);
    if (!configObject.contains(kConfigGroupAppAttribute))
        return false;

    auto appAttr = configObject[kConfigGroupAppAttribute].toObject();
    if (!appAttr.contains(kConfigKeyIconSizeLevel))
        return false;

    appAttr[kConfigKeyIconSizeLevel] = newIconSizeLevel;
    configObject[kConfigGroupAppAttribute] = appAttr;

    return writeConfigFile();
}

void AppAttributeUpgradeUnit::completed()
{
    qCInfo(logToolUpgrade) << "application attribute upgrade completed.";
}

int AppAttributeUpgradeUnit::transIconSizeLevel(int oldIconSizeLevel) const
{
    switch (oldIconSizeLevel) {
    case 0:
        return 3;
    case 1:
        return 5;
    case 2:
        return 9;
    case 3:
        return 13;
    case 4:
        return 17;
    default:
        return oldIconSizeLevel;
    }
}

bool AppAttributeUpgradeUnit::backupAppAttribute() const
{
    QDir backupDir(kBackupDirPath);
    if (!backupDir.exists()) {
        if (!backupDir.mkpath(".")) {
            qCWarning(logToolUpgrade) << "upgrade: create backup directory failed: " << kBackupDirPath;
            return false;
        }
    }

    QString backupFilePath { kBackupDirPath + "/" + kConfigGroupAppAttribute + "_" + kAppAttributeVersion + ".backup" };
    if (QFile::exists(backupFilePath)) {
        qCWarning(logToolUpgrade) << "upgrade: backup file already exists: " << backupFilePath;
        return false;
    }

    if (!QFile::copy(kConfigurationPath, backupFilePath)) {
        qCWarning(logToolUpgrade) << "upgrade: copy file failed: " << kConfigurationPath << " to " << backupFilePath;
        return false;
    }

    return true;
}

bool AppAttributeUpgradeUnit::writeConfigFile() const
{
    QJsonDocument newDoc(configObject);
    QByteArray data = newDoc.toJson();

    QFile writeFile(kConfigurationPath);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(logToolUpgrade) << "upgrade: open file failed: " << kConfigurationPath;
        return false;
    }

    writeFile.write(data);
    writeFile.close();

    return true;
}
