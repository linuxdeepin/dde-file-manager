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

namespace GlobalServerDefines {

/*!
 * \brief Content categories
 * for 'Device Busy'-related messages sent to the Notification Center
 */
namespace DeviceBusyAction {
const int kSafelyRemove { 0 };
const int kUnmount { 1 };
const int kRemove { 2 };
const int kEject { 3 };
}   // namespace DeviceBusyAction

/*!
 * \brief Property names of the device,
 *  only the names of the properties that need to be monitored
 * for changes are listed here
 */
namespace DeviceAttribute {
const char *const kIdLabel { "IdLabel" };
const char *const kMountPoints { "MountPoints" };
const char *const kBlockSize { "BlockSize" };
const char *const kSize { "Size" };
const char *const kCleartextDevice { "CleartextDevice" };
}   // namespace DeviceAttribute

/*!
 * \brief Device property information
 */
namespace DeviceProperty {
// common
const char *const kId { "Id" };
const char *const kMountPoint { "MountPoint" };
const char *const kFileSystem { "IdType" };
const char *const kSizeTotal { "SizeTotal" };
const char *const kSizeFree { "SizeFree" };
const char *const kSizeUsed { "SizeUsed" };

// block
const char *const kUUID { "IdUUID" };
const char *const kFsVersion { "IdVersion" };
const char *const kDevice { "Device" };
const char *const kIdLabel { "IdLabel" };
const char *const kMedia { "Media" };
const char *const kReadOnly { "ReadOnly" };
const char *const kRemovable { "Removable" };
const char *const kMediaRemovable { "MediaRemovable" };
const char *const kOptical { "Optical" };
const char *const kOpticalDrive { "OpticalDrive" };
const char *const kOpticalBlank { "OpticalBlank" };
const char *const kMediaAvailable { "MediaAvailable" };
const char *const kCanPowerOff { "CanPowerOff" };
const char *const kEjectable { "Ejectable" };
const char *const kIsEncrypted { "IsEncrypted" };
const char *const kIsLoopDevice { "IsLoopDevice" };
const char *const kHasFileSystem { "HasFileSystem" };
const char *const kHasPartitionTable { "HasPartitionTable" };
const char *const kHasPartition { "HasPartition" };
const char *const kHasExtendedPatition { "HasExtendedPartition" };
const char *const kHintSystem { "HintSystem" };
const char *const kHintIgnore { "HintIgnore" };
const char *const kCryptoBackingDevice { "CryptoBackingDevice" };
const char *const kDrive { "Drive" };
const char *const kMountPoints { "MountPoints" };
const char *const kMediaCompatibility { "MediaCompatibility" };
const char *const kCleartextDevice { "CleartextDevice" };

const char *const kDisplayName { "DisplayName" };
const char *const kDeviceIcon { "DeviceIcon" };

}   // namespace DeviceProperty

/*!
 * \brief Options for processing the device list interface,
 * returning a list of devices with different contents
 * detail in: DeviceService::blockDevicesIdList
 */
namespace ListOpt {
const char *const kUnmountable { "unmountable" };
const char *const kMountable { "mountable" };
const char *const kNotIgnorable { "not_ignorable" };
}   // namespace DeviceProperty

/*!
 * \brief Options for mount network device, such as smb, ftp etc.
 * detail in DeviceService::mountNetworkDevice
 */
namespace NetworkMountParamKey {
const char *const kUser { "user" };
const char *const kDomain { "domain" };
const char *const kPasswd { "passwd" };
const char *const kPasswdSaveMode { "savePasswd" };
}   // namespace NetworkMountParamKey

}   // namespace GlobalServerDefines

#endif   // GLOBAL_SERVER_DEFINES_H
