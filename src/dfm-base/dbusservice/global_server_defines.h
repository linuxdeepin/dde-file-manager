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
constexpr int kSafelyRemove { 0 };
constexpr int kUnmount { 1 };
constexpr int kRemove { 2 };
constexpr int kEject { 3 };
}   // namespace DeviceBusyAction

/*!
 * \brief Property names of the device,
 *  only the names of the properties that need to be monitored
 * for changes are listed here
 */
namespace DeviceAttribute {
constexpr char kIdLabel[] { "IdLabel" };
constexpr char kMountPoints[] { "MountPoints" };
constexpr char kBlockSize[] { "BlockSize" };
constexpr char kSize[] { "Size" };
constexpr char kCleartextDevice[] { "CleartextDevice" };
}   // namespace DeviceAttribute

/*!
 * \brief Device property information
 */
namespace DeviceProperty {
// common
constexpr char kId[] { "Id" };
constexpr char kMountPoint[] { "MountPoint" };
constexpr char kFileSystem[] { "IdType" };
constexpr char kSizeTotal[] { "SizeTotal" };
constexpr char kSizeFree[] { "SizeFree" };
constexpr char kSizeUsed[] { "SizeUsed" };

// block
constexpr char kUUID[] { "IdUUID" };
constexpr char kFsVersion[] { "IdVersion" };
constexpr char kDevice[] { "Device" };
constexpr char kIdLabel[] { "IdLabel" };
constexpr char kMedia[] { "Media" };
constexpr char kReadOnly[] { "ReadOnly" };
constexpr char kRemovable[] { "Removable" };
constexpr char kMediaRemovable[] { "MediaRemovable" };
constexpr char kOptical[] { "Optical" };
constexpr char kOpticalDrive[] { "OpticalDrive" };
constexpr char kOpticalBlank[] { "OpticalBlank" };
constexpr char kOpticalMediaType[] { "OpticalMediaType" };
constexpr char kOpticalWriteSpeed[] { "OpticalWriteSpeed" };
constexpr char kMediaAvailable[] { "MediaAvailable" };
constexpr char kCanPowerOff[] { "CanPowerOff" };
constexpr char kEjectable[] { "Ejectable" };
constexpr char kIsEncrypted[] { "IsEncrypted" };
constexpr char kIsLoopDevice[] { "IsLoopDevice" };
constexpr char kHasFileSystem[] { "HasFileSystem" };
constexpr char kHasPartitionTable[] { "HasPartitionTable" };
constexpr char kHasPartition[] { "HasPartition" };
constexpr char kHasExtendedPatition[] { "HasExtendedPartition" };
constexpr char kHintSystem[] { "HintSystem" };
constexpr char kHintIgnore[] { "HintIgnore" };
constexpr char kCryptoBackingDevice[] { "CryptoBackingDevice" };
constexpr char kDrive[] { "Drive" };
constexpr char kMountPoints[] { "MountPoints" };
constexpr char kMediaCompatibility[] { "MediaCompatibility" };
constexpr char kCleartextDevice[] { "CleartextDevice" };
constexpr char kDisplayName[] { "DisplayName" };
constexpr char kDeviceIcon[] { "DeviceIcon" };
constexpr char kConnectionBus[] { "connectBus" };

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
};
Q_ENUMS(DeviceQueryOption)
Q_DECLARE_FLAGS(DeviceQueryOptions, DeviceQueryOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceQueryOptions)

/*!
 * \brief Options for mount network device, such as smb, ftp etc.
 * detail in DeviceService::mountNetworkDevice
 */
namespace NetworkMountParamKey {
constexpr char kUser[] { "user" };
constexpr char kDomain[] { "domain" };
constexpr char kPasswd[] { "passwd" };
constexpr char kPasswdSaveMode[] { "savePasswd" };
constexpr char kAnonymous[] { "anonymous" };
constexpr char kMessage[] { "message" };
}   // namespace NetworkMountParamKey

}   // namespace GlobalServerDefines

#endif   // GLOBAL_SERVER_DEFINES_H
