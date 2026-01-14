// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H
#define DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H

#include "services/diskencrypt/globaltypesdefine.h"

#include <dfm-base/dfm_log_defines.h>

#include <QtCore/qglobal.h>
#include <QDir>
#include <QString>
#include <QDBusInterface>
#include <QDBusConnection>

#if defined(DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
#    define DFMPLUGIN_DISK_ENCRYPT_EXPORT Q_DECL_EXPORT
#else
#    define DFMPLUGIN_DISK_ENCRYPT_EXPORT Q_DECL_IMPORT
#endif

#define DISKENC_NAMESPACE dfmplugin_diskenc

enum TPMModuleEncType {
    kUnknow = 0,
    kUseTpmAndPcr,
    kUseTpmAndPin,
    kUseTpmAndPcrAndPin
};

inline constexpr char kDaemonBusName[] { "org.deepin.Filemanager.DiskEncrypt" };
inline constexpr char kDaemonBusPath[] { "/org/deepin/Filemanager/DiskEncrypt" };
inline constexpr char kDaemonBusIface[] { "org.deepin.Filemanager.DiskEncrypt" };

inline constexpr char kMenuPluginName[] { "dfmplugin_menu" };
inline constexpr char kComputerMenuSceneName[] { "ComputerMenu" };

inline constexpr int kPasswordSize { 14 };
inline const QString kGlobalTPMConfigPath("/tmp/dfm-encrypt");

inline constexpr char kTPMSessionHashAlgo[] { "sha256" };
inline constexpr char kTPMSessionKeyAlgo[] { "aes" };
inline constexpr char kTPMPrimaryHashAlgo[] { "sha256" };
inline constexpr char kTPMPrimaryKeyAlgo[] { "rsa" };
inline constexpr char kTPMMinorHashAlgo[] { "sha256" };
inline constexpr char kTPMMinorKeyAlgo[] { "aes" };

inline constexpr char kTCMSessionHashAlgo[] { "sm3_256" };
inline constexpr char kTCMSessionKeyAlgo[] { "sm4" };
inline constexpr char kTCMPrimaryHashAlgo[] { "sm3_256" };
inline constexpr char kTCMPrimaryKeyAlgo[] { "sm4" };
inline constexpr char kTCMMinorHashAlgo[] { "sm3_256" };
inline constexpr char kTCMMinorKeyAlgo[] { "sm4" };

inline constexpr char kPcr[] { "0,7" };
inline constexpr char kTPMPcrBank[] { "sha256" };
inline constexpr char kTCMPcrBank[] { "sm3_256" };

// DBus timeout constant: 2 seconds to prevent UI blocking when service is unavailable
inline constexpr int kDiskEncryptDBusTimeoutMs = 2000;

// TPM Control service constants
inline constexpr char kTPMControlService[] = "org.deepin.Filemanager.TPMControl";
inline constexpr char kTPMControlPath[] = "/org/deepin/Filemanager/TPMControl";
inline constexpr char kTPMControlInterface[] = "org.deepin.Filemanager.TPMControl";

/**
 * @brief Setup QDBusInterface with standardized timeout
 * @param iface DBus interface to configure
 *
 * This helper ensures consistent timeout configuration across all DBus calls
 * and centralizes the timeout value for easy maintenance.
 */
inline void setupDbusInterface(QDBusInterface &iface)
{
    if (iface.isValid()) {
        iface.setTimeout(kDiskEncryptDBusTimeoutMs);
    }
}

/**
 * @brief Helper macros for creating DBus interfaces with timeout
 * Usage:
 *   CREATE_DBUS_INTERFACE(iface, service, path, interface)
 *   CREATE_TPM_INTERFACE(iface)
 */
#define CREATE_DBUS_INTERFACE(name, service, path, interface) \
    QDBusInterface name(service, path, interface, QDBusConnection::systemBus());

#define CREATE_TPM_INTERFACE(name) \
    CREATE_DBUS_INTERFACE(name, kTPMControlService, kTPMControlPath, kTPMControlInterface)

#define CREATE_DAEMON_INTERFACE(name) \
    CREATE_DBUS_INTERFACE(name, kDaemonBusName, kDaemonBusPath, kDaemonBusIface)

DFM_LOG_USE_CATEGORY(DISKENC_NAMESPACE)

#endif   // DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H
