/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
inline constexpr int kSafelyRemove { 0 };
inline constexpr int kUnmount { 1 };
inline constexpr int kRemove { 2 };
inline constexpr int kEject { 3 };
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
inline constexpr char kConnectionBus[] { "connectBus" };
inline constexpr char kUDisks2Size[] { "UDisks2Size" };
}   // namespace DeviceProperty

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
