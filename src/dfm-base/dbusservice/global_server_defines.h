// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_SERVER_DEFINES_H
#define GLOBAL_SERVER_DEFINES_H

#include <QObject>

namespace GlobalServerDefines {
Q_NAMESPACE
/*!
 * \brief Content categories
 * for 'Device Busy'-related messages sent to the Notification Center
 */
namespace DeviceBusyAction {
inline constexpr char kUnmount[] { "unmount" };
inline constexpr char kPowerOff[] { "poweroff" };
inline constexpr char kEject[] { "eject" };
}   // namespace DeviceBusyAction

/*!
 * \brief Property names of the device,
 *  only the names of the properties that need to be monitored
 * for changes are listed here
 */
namespace DeviceAttribute {
inline constexpr char kIdLabel[] { "IdLabel" };
inline constexpr char kMountPoints[] { "MountPoints" };
inline constexpr char kBlockSize[] { "BlockSize" };
inline constexpr char kSize[] { "Size" };
inline constexpr char kCleartextDevice[] { "CleartextDevice" };
}   // namespace DeviceAttribute

/*!
 * \brief Device property information
 */
namespace DeviceProperty {
// common
inline constexpr char kId[] { "Id" };
inline constexpr char kMountPoint[] { "MountPoint" };
inline constexpr char kFileSystem[] { "IdType" };
inline constexpr char kSizeTotal[] { "SizeTotal" };
inline constexpr char kSizeFree[] { "SizeFree" };
inline constexpr char kSizeUsed[] { "SizeUsed" };

// block
inline constexpr char kUUID[] { "IdUUID" };
inline constexpr char kFsVersion[] { "IdVersion" };
inline constexpr char kDevice[] { "Device" };
inline constexpr char kIdLabel[] { "IdLabel" };
inline constexpr char kMedia[] { "Media" };
inline constexpr char kReadOnly[] { "ReadOnly" };
inline constexpr char kRemovable[] { "Removable" };
inline constexpr char kMediaRemovable[] { "MediaRemovable" };
inline constexpr char kOptical[] { "Optical" };
inline constexpr char kOpticalDrive[] { "OpticalDrive" };
inline constexpr char kOpticalBlank[] { "OpticalBlank" };
inline constexpr char kOpticalMediaType[] { "OpticalMediaType" };
inline constexpr char kOpticalWriteSpeed[] { "OpticalWriteSpeed" };
inline constexpr char kMediaAvailable[] { "MediaAvailable" };
inline constexpr char kCanPowerOff[] { "CanPowerOff" };
inline constexpr char kEjectable[] { "Ejectable" };
inline constexpr char kIsEncrypted[] { "IsEncrypted" };
inline constexpr char kIsLoopDevice[] { "IsLoopDevice" };
inline constexpr char kHasFileSystem[] { "HasFileSystem" };
inline constexpr char kHasPartitionTable[] { "HasPartitionTable" };
inline constexpr char kHasPartition[] { "HasPartition" };
inline constexpr char kHasExtendedPatition[] { "HasExtendedPartition" };
inline constexpr char kHintSystem[] { "HintSystem" };
inline constexpr char kHintIgnore[] { "HintIgnore" };
inline constexpr char kCryptoBackingDevice[] { "CryptoBackingDevice" };
inline constexpr char kDrive[] { "Drive" };
inline constexpr char kMountPoints[] { "MountPoints" };
inline constexpr char kMediaCompatibility[] { "MediaCompatibility" };
inline constexpr char kCleartextDevice[] { "CleartextDevice" };
inline constexpr char kDisplayName[] { "DisplayName" };
inline constexpr char kDeviceIcon[] { "DeviceIcon" };
inline constexpr char kConnectionBus[] { "ConnectionBus" };
inline constexpr char kUDisks2Size[] { "UDisks2Size" };
inline constexpr char kDriveModel[] { "DriveModel" };
inline constexpr char kPreferredDevice[] { "PreferredDevice" };
}   // namespace DeviceProperty

/*!
 * brief Recent property information
 */
namespace RecentProperty {
inline constexpr char kPath[] { "Path" };
inline constexpr char kHref[] { "Href" };
inline constexpr char kModified[] { "modified" };

inline constexpr char kAppName[] { "AppName" };
inline constexpr char kAppExec[] { "AppExec" };
inline constexpr char kMimeType[] { "MimeType" };
}   // namespace RecentProperty

/*!
 * \brief Options for processing the device list interface,
 * returning a list of devices with different contents
 * detail in: DeviceService::blockDevicesIdList
 */
enum DeviceQueryOption {
    kNoCondition = 0,
    kMountable = 1,
    kMounted = 1 << 1,
    kRemovable = 1 << 2,
    kNotIgnored = 1 << 3,
    kNotMounted = 1 << 4,
    kOptical = 1 << 5,
    kSystem = 1 << 6,
    kLoop = 1 << 7,
};
Q_ENUMS(DeviceQueryOption)
Q_DECLARE_FLAGS(DeviceQueryOptions, DeviceQueryOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceQueryOptions)

/*!
 * \brief Options for mount network device, such as smb, ftp etc.
 * detail in DeviceService::mountNetworkDevice
 */
namespace NetworkMountParamKey {
inline constexpr char kUser[] { "user" };
inline constexpr char kDomain[] { "domain" };
inline constexpr char kPasswd[] { "passwd" };
inline constexpr char kPasswdSaveMode[] { "savePasswd" };
inline constexpr char kAnonymous[] { "anonymous" };
inline constexpr char kMessage[] { "message" };
}   // namespace NetworkMountParamKey

}   // namespace GlobalServerDefines

#endif   // GLOBAL_SERVER_DEFINES_H
