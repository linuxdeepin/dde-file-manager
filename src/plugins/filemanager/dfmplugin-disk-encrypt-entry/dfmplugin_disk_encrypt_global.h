// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H
#define DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H

#include "services/diskencrypt/globaltypesdefine.h"

#include <QtCore/qglobal.h>
#include <QDir>
#include <QString>

#if defined(DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
#    define DFMPLUGIN_DISK_ENCRYPT_EXPORT Q_DECL_EXPORT
#else
#    define DFMPLUGIN_DISK_ENCRYPT_EXPORT Q_DECL_IMPORT
#endif

enum TPMModuleEncType {
    kUnknow = 0,
    kUseTpmAndPcr,
    kUseTpmAndPin,
    kUseTpmAndPrcAndPin
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
inline constexpr char kConfigKeySessionHashAlgo[] { "session_hash_algo" };
inline constexpr char kConfigKeySessionKeyAlgo[] { "session_key_algo" };
inline constexpr char kConfigKeyPriHashAlgo[] { "primary_hash_algo" };
inline constexpr char kConfigKeyPriKeyAlgo[] { "primary_key_algo" };

#endif   // DFMPLUGIN_DISK_ENCRYPT_GLOBAL_H
