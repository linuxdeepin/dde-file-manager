// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SYSTEMDCPUUTILS_H
#define SYSTEMDCPUUTILS_H

#include "service_textindex_global.h"

#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace SystemdCpuUtils {

/**
 * @brief Sets CPU quota for a specified systemd user service.
 * @param serviceName Full service name (e.g. "deepin-service-plugin@org.deepin.Filemanager.TextIndex.service")
 * @param percentage CPU quota percentage (e.g. 50 for 50% limit)
 * @param[out] errorMsg If execution fails, contains the error message
 * @return Returns true if command executed successfully, false otherwise
 */
bool setCpuQuota(const QString &serviceName, int percentage, QString *errorMsg);

/**
 * @brief Removes CPU quota restrictions for a specified systemd user service.
 * @param serviceName Full service name
 * @param[out] errorMsg If execution fails, contains the error message
 * @return Returns true if command executed successfully, false otherwise
 */
bool resetCpuQuota(const QString &serviceName, QString *errorMsg);

/**
 * @brief Acquires a CPU quota slot for a limited task (Light/Medium/Heavy).
 *        Uses an atomic reference counter: only the first caller actually sets the quota.
 * @param serviceName Full service name
 * @param percentage CPU quota percentage
 */
void acquireCpuQuota(const QString &serviceName, int percentage);

/**
 * @brief Releases a CPU quota slot for a limited task.
 *        Only the last caller (counter drops to 0) actually resets the quota.
 * @param serviceName Full service name
 */
void releaseCpuQuota(const QString &serviceName);

/**
 * @brief Handles CPU quota for a Manual task (start or finish).
 *        Resets the quota only when no limited tasks are running.
 * @param serviceName Full service name
 */
void manualTaskCpuQuota(const QString &serviceName);

}   // namespace SystemdCpuUtils

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SYSTEMDCPUUTILS_H
