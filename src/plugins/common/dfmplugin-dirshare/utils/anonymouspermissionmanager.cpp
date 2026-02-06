// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anonymouspermissionmanager.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QSaveFile>

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE

DPDIRSHARE_BEGIN_NAMESPACE

AnonymousPermissionManager *AnonymousPermissionManager::instance()
{
    static AnonymousPermissionManager manager;
    return &manager;
}

AnonymousPermissionManager::AnonymousPermissionManager(QObject *parent)
    : QObject(parent)
{
    // Use XDG Config specification path
    configFilePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/deepin/dde-file-manager/anonymous-share-permissions.json";
    loadFromFile();
}

bool AnonymousPermissionManager::setAnonymousPermissions(const QString &filePath, DirectoryType type, bool writable)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        fmWarning() << "File does not exist:" << filePath;
        return false;
    }

    QFile::Permissions current = fileInfo.permissions();

    if (type == DirectoryType::kHomeDirectory) {
        // Record home directory permissions (only on first anonymous share)
        if (homeRecord.path.isEmpty()) {
            homeRecord.path = filePath;
            homeRecord.original = current;
            homeRecord.expected = getHomeExpectedPermissions(current);
            fmInfo() << "Recorded home directory:" << filePath
                     << "original:" << QString::number(homeRecord.original, 8);
        }

        // Set home directory permissions (add read and execute for others)
        QFile file(filePath);
        if (!file.setPermissions(homeRecord.expected)) {
            fmWarning() << "Failed to set home directory permissions:" << filePath;
            return false;
        }

        fmInfo() << "Set home directory permissions for anonymous share:" << filePath
                 << "from" << QString::number(current, 8)
                 << "to" << QString::number(homeRecord.expected, 8);
    } else {
        // Record shared directory permissions
        PermissionRecord record;
        record.original = current;

        // Calculate expected permissions based on writable flag
        if (writable) {
            record.expected = getExpectedPermissions(current);
        } else {
            // For read-only anonymous share, only add execute permissions
            using P = QFile::Permission;
            record.expected = current | P::ExeGroup | P::ExeOther;
        }

        shareRecords[filePath] = record;
        fmInfo() << "Recorded shared directory:" << filePath
                 << "original:" << QString::number(record.original, 8)
                 << "expected:" << QString::number(record.expected, 8);

        // Set shared directory permissions
        QFile file(filePath);
        if (!file.setPermissions(record.expected)) {
            fmWarning() << "Failed to set shared directory permissions:" << filePath;
            shareRecords.remove(filePath);
            return false;
        }

        fmInfo() << "Set shared directory permissions for anonymous share:" << filePath
                 << "from" << QString::number(current, 8)
                 << "to" << QString::number(record.expected, 8);
    }

    return saveToFile();
}

bool AnonymousPermissionManager::restoreDirectoryPermissions(const QString &filePath)
{
    // Only proceed if has record (record exists means it was anonymous share)
    if (!shareRecords.contains(filePath))
        return false;

    const auto &record = shareRecords[filePath];
    QFile::Permissions current = QFileInfo(filePath).permissions();

    // Check if current permissions match expected permissions
    // If not equal, user has manually modified permissions
    if (current != record.expected) {
        fmInfo() << "Directory permissions modified by user, skip restore:" << filePath
                 << "expected:" << QString::number(record.expected, 8)
                 << "current:" << QString::number(current, 8);
        // Clean record to prevent incorrect restore
        cleanRecord(filePath);
        return false;
    }

    // Restore to original permissions
    QFile file(filePath);
    if (file.setPermissions(record.original)) {
        fmInfo() << "Successfully restored directory permissions:" << filePath
                 << "from" << QString::number(current, 8)
                 << "to" << QString::number(record.original, 8);
        cleanRecord(filePath);
        return true;
    }

    fmWarning() << "Failed to restore directory permissions:" << filePath;
    return false;
}

bool AnonymousPermissionManager::restoreHomeDirectoryIfNoAnonymousShares()
{
    // No home directory record, nothing to do
    if (homeRecord.path.isEmpty())
        return false;

    int currentCount = getCurrentAnonymousShareCount();

    // Do not restore if anonymous shares still exist
    if (currentCount > 0) {
        fmDebug() << "Still has" << currentCount << "anonymous shares, skip restore home directory";
        return false;
    }

    QFile::Permissions current = QFileInfo(homeRecord.path).permissions();

    // Check if user modified home directory permissions
    if (current != homeRecord.expected) {
        fmInfo() << "Home directory permissions modified by user, skip restore:"
                 << "expected:" << QString::number(homeRecord.expected, 8)
                 << "current:" << QString::number(current, 8);
        // Clear home record
        homeRecord = HomeDirectoryRecord();
        saveToFile();
        return false;
    }

    // Restore home directory permissions
    QFile file(homeRecord.path);
    if (file.setPermissions(homeRecord.original)) {
        fmInfo() << "Successfully restored home directory permissions"
                 << "from" << QString::number(current, 8)
                 << "to" << QString::number(homeRecord.original, 8);
        // Clear home record
        homeRecord = HomeDirectoryRecord();
        saveToFile();
        return true;
    }

    fmWarning() << "Failed to restore home directory permissions:" << homeRecord.path;
    return false;
}

void AnonymousPermissionManager::cleanRecord(const QString &filePath)
{
    shareRecords.remove(filePath);
    saveToFile();
}

QFile::Permissions AnonymousPermissionManager::getExpectedPermissions(QFile::Permissions original) const
{
    using P = QFile::Permission;
    // Anonymous share requires: group write/execute, others write/execute
    return original | P::WriteGroup | P::ExeGroup | P::WriteOther | P::ExeOther;
}

QFile::Permissions AnonymousPermissionManager::getHomeExpectedPermissions(QFile::Permissions original) const
{
    using P = QFile::Permission;
    // Home directory requires: others read/execute
    return original | P::ReadOther | P::ExeOther;
}

int AnonymousPermissionManager::getCurrentAnonymousShareCount() const
{
    // Get anonymous share count from recorded shares in JSON file
    return shareRecords.size();
}

bool AnonymousPermissionManager::loadFromFile()
{
    QFile file(configFilePath);
    if (!file.exists()) {
        // First run, ensure config directory exists
        QDir().mkpath(QFileInfo(configFilePath).path());
        fmInfo() << "Config file does not exist, will be created on first save";
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "Failed to open config file for reading:" << configFilePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        fmWarning() << "Invalid JSON config file:" << configFilePath;
        file.close();
        return false;
    }

    QJsonObject root = doc.object();

    // Load shared directory records
    QJsonObject sharesObj = root[kJsonShares].toObject();
    for (const QString &path : sharesObj.keys()) {
        QJsonObject shareObj = sharesObj[path].toObject();
        PermissionRecord record;
        record.original = static_cast<QFile::Permissions>(shareObj[kJsonOriginal].toInt());
        record.expected = static_cast<QFile::Permissions>(shareObj[kJsonExpected].toInt());
        shareRecords[path] = record;
    }

    // Load home directory record
    QJsonObject homeObj = root[kJsonHomeDirectory].toObject();
    if (!homeObj.isEmpty()) {
        homeRecord.path = homeObj[kJsonPath].toString();
        homeRecord.original = static_cast<QFile::Permissions>(homeObj[kJsonOriginal].toInt());
        homeRecord.expected = static_cast<QFile::Permissions>(homeObj[kJsonExpected].toInt());
    }

    file.close();
    fmInfo() << "Loaded permission records from config file, shares:" << shareRecords.size();
    return true;
}

bool AnonymousPermissionManager::saveToFile()
{
    // Ensure directory exists
    QDir().mkpath(QFileInfo(configFilePath).path());

    QSaveFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        fmWarning() << "Failed to open config file for writing:" << configFilePath;
        return false;
    }

    QJsonObject root;
    root[kJsonVersion] = 1;

    // Save shared directory records
    QJsonObject sharesObj;
    for (const QString &path : shareRecords.keys()) {
        const auto &record = shareRecords[path];
        QJsonObject shareObj;
        shareObj[kJsonOriginal] = static_cast<int>(record.original);
        shareObj[kJsonExpected] = static_cast<int>(record.expected);
        sharesObj[path] = shareObj;
    }
    root[kJsonShares] = sharesObj;

    // Save home directory record (only if exists)
    if (!homeRecord.path.isEmpty()) {
        QJsonObject homeObj;
        homeObj[kJsonPath] = homeRecord.path;
        homeObj[kJsonOriginal] = static_cast<int>(homeRecord.original);
        homeObj[kJsonExpected] = static_cast<int>(homeRecord.expected);
        root[kJsonHomeDirectory] = homeObj;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    // Commit changes atomically
    if (!file.commit()) {
        fmWarning() << "Failed to save config file:" << configFilePath;
        return false;
    }

    return true;
}

DPDIRSHARE_END_NAMESPACE
