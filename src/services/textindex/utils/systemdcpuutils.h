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

}   // namespace SystemdCpuUtils

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SYSTEMDCPUUTILS_H
