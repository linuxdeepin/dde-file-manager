// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_USBREPAIR_GLOBAL_H
#define SERVICE_USBREPAIR_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#include <QString>
#include <QStringList>

#define SERVICEUSBREPAIR_NAMESPACE service_usbrepair
#define SERVICEUSBREPAIR_BEGIN_NAMESPACE namespace SERVICEUSBREPAIR_NAMESPACE {
#define SERVICEUSBREPAIR_END_NAMESPACE }
#define SERVICEUSBREPAIR_USE_NAMESPACE using namespace SERVICEUSBREPAIR_NAMESPACE;

SERVICEUSBREPAIR_BEGIN_NAMESPACE

namespace Defines {

// D-Bus
inline const QString kServiceName = QLatin1String("org.deepin.Filemanager.UsbRepair");
inline constexpr char kServiceObjPath[] { "/org/deepin/Filemanager/UsbRepair" };

// PolKit
inline const QString kPolkitActionId = QLatin1String("org.deepin.filemanager.usbrepair.repair");

// Filesystem whitelist
inline const QStringList kSupportedFsTypes = { "vfat", "exfat", "ntfs", "ext4" };

// Timeout (ms)
inline constexpr int kFsckTimeoutMs { 300000 };   // 300s

// Monitor delay (ms)
inline constexpr int kMountSettleDelayMs { 3000 };   // 3 seconds - wait for auto-mount to complete

// Error types
inline const QString kErrorTypeDirtyBit = QLatin1String("dirty_bit");
inline const QString kErrorTypeMountFailed = QLatin1String("mount_failed");
inline const QString kErrorTypeReadOnly = QLatin1String("read_only");

// udisks2
inline const QString kUdisks2Service = QLatin1String("org.freedesktop.UDisks2");
inline const QString kUdisks2Path = QLatin1String("/org/freedesktop/UDisks2");
inline const QString kUdisks2ObjectMgrIface = QLatin1String("org.freedesktop.DBus.ObjectManager");
inline const QString kUdisks2BlockIface = QLatin1String("org.freedesktop.UDisks2.Block");
inline const QString kUdisks2FilesystemIface = QLatin1String("org.freedesktop.UDisks2.Filesystem");
inline const QString kUdisks2DriveIface = QLatin1String("org.freedesktop.UDisks2.Drive");

}   // namespace Defines

DFM_LOG_USE_CATEGORY(SERVICEUSBREPAIR_NAMESPACE)

SERVICEUSBREPAIR_END_NAMESPACE

#endif   // SERVICE_USBREPAIR_GLOBAL_H
