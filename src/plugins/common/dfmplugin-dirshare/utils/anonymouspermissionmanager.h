// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANONYMOUSPERMISSIONMANAGER_H
#define ANONYMOUSPERMISSIONMANAGER_H

#include <QObject>
#include <QFile>
#include <QMap>
#include <QFileInfo>

#include "dfmplugin_dirshare_global.h"

DPDIRSHARE_BEGIN_NAMESPACE

class AnonymousPermissionManager : public QObject
{
    Q_OBJECT

public:
    // JSON configuration file keys
    static constexpr char kJsonVersion[] = "version";
    static constexpr char kJsonShares[] = "shares";
    static constexpr char kJsonHomeDirectory[] = "homeDirectory";
    static constexpr char kJsonPath[] = "path";
    static constexpr char kJsonOriginal[] = "original";
    static constexpr char kJsonExpected[] = "expected";

    /**
     * @brief Directory type enum for clear interface
     */
    enum class DirectoryType
    {
        kSharedDirectory,
        kHomeDirectory
    };

    /**
     * @brief Permission record structure
     */
    struct PermissionRecord
    {
        QFile::Permissions original;
        QFile::Permissions expected;
    };

    /**
     * @brief Home directory record structure
     */
    struct HomeDirectoryRecord
    {
        QString path;
        QFile::Permissions original;
        QFile::Permissions expected;
    };

    static AnonymousPermissionManager *instance();

    /**
     * @brief Set directory permissions for anonymous share
     * This method records original permissions and sets new permissions
     * @param filePath File path
     * @param type Directory type (shared directory or home directory)
     * @param writable Whether share is writable (only for shared directories)
     * @return Success
     */
    bool setAnonymousPermissions(const QString &filePath, DirectoryType type, bool writable = true);

    /**
     * @brief Restore directory permissions (from anonymous share back to original)
     * @param filePath File path
     * @return Success
     */
    bool restoreDirectoryPermissions(const QString &filePath);

    /**
     * @brief Restore home directory permissions when no anonymous shares exist
     * @return Success
     */
    bool restoreHomeDirectoryIfNoAnonymousShares();

    /**
     * @brief Clean permission record for specified path
     * @param filePath File path
     */
    void cleanRecord(const QString &filePath);

private:
    explicit AnonymousPermissionManager(QObject *parent = nullptr);

    bool loadFromFile();
    bool saveToFile();

    /**
     * @brief Calculate expected permissions for anonymous shared directory
     * (add write and execute permissions for group and others)
     */
    QFile::Permissions getExpectedPermissions(QFile::Permissions original) const;

    /**
     * @brief Calculate expected permissions for home directory
     * (add read and execute permissions for others)
     */
    QFile::Permissions getHomeExpectedPermissions(QFile::Permissions original) const;

    /**
     * @brief Get current anonymous share count from UserShareHelper
     */
    int getCurrentAnonymousShareCount() const;

private:
    QString configFilePath;
    QMap<QString, PermissionRecord> shareRecords;
    HomeDirectoryRecord homeRecord;
};

DPDIRSHARE_END_NAMESPACE

#endif   // ANONYMOUSPERMISSIONMANAGER_H
