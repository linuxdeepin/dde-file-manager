// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEALIASMANAGER_H
#define DEVICEALIASMANAGER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QReadWriteLock>

namespace dfmbase {

/**
 * @brief Network protocol device alias manager
 *
 * This class manages aliases for network protocol devices (SMB, FTP, SFTP, etc.).
 * It provides persistent storage using Application::genericSetting() and uses
 * separated protocol identifiers (scheme:host:port) as unique keys.
 */
class NPDeviceAliasManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static NPDeviceAliasManager *instance();

    /**
     * @brief Get alias for a protocol URL
     * @param protocolUrl The protocol URL (e.g., "smb://10.10.10.10/share")
     * @return The alias string, empty if no alias is set
     */
    QString getAlias(const QUrl &protocolUrl) const;

    /**
     * @brief Set alias for a protocol URL
     * @param protocolUrl The protocol URL to set alias for
     * @param alias The alias string to set. If empty, the alias will be removed.
     * @return true if the operation succeeded, false if URL is invalid or unsupported
     *
     * @details This method will:
     * - Add a new alias entry if it doesn't exist
     * - Update existing alias if it already exists
     * - Remove alias entry if alias string is empty
     * - Persist changes to application settings
     */
    bool setAlias(const QUrl &protocolUrl, const QString &alias);

    /**
     * @brief Remove alias for a protocol URL
     * @param protocolUrl The protocol URL to remove alias for
     *
     * @details This is a convenience method that calls setAlias() with an empty string.
     * The operation will persist changes to application settings.
     */
    void removeAlias(const QUrl &protocolUrl);

    /**
     * @brief Check if a protocol URL has an alias
     * @param protocolUrl The protocol URL to check
     * @return true if alias exists, false otherwise
     */
    bool hasAlias(const QUrl &protocolUrl) const;

    /**
     * @brief Check if alias can be set for the given protocol URL
     * @param protocolUrl The protocol URL to validate
     * @return true if the URL is valid and scheme is supported for alias setting, false otherwise
     *
     * @details This method validates:
     * - URL is valid and has a non-empty host
     * - Protocol scheme is in the supported list (smb, ftp, sftp, etc.)
     */
    bool canSetAlias(const QUrl &protocolUrl) const;

private:
    /**
     * @brief Private constructor for singleton pattern
     * @param parent Parent QObject
     */
    explicit NPDeviceAliasManager(QObject *parent = nullptr);

    QUrl convertToProtocolUrl(const QUrl &url) const;

    mutable QReadWriteLock rwLock;
};

}

#endif   // DEVICEALIASMANAGER_H
