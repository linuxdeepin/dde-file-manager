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
const char *const kId { "id" };
const char *const kMountpoint { "mountpoint" };
const char *const kFilesystem { "filesystem" };
const char *const kSizeTotal { "size_total" };
const char *const kSizeFree { "size_free" };
const char *const kSizeUsed { "size_usage" };

// block
const char *const kUUID { "uuid" };
const char *const kFsVersion { "fs_version" };
const char *const kDevice { "device" };
const char *const kIdLabel { "id_label" };
const char *const kMedia { "media" };
const char *const kReadOnly { "read_only" };
const char *const kRemovable { "removable" };
const char *const kMediaRemovable { "media_removable" };
const char *const kOptical { "optical" };
const char *const kOpticalDrive { "optical_drive" };
const char *const kOpticalBlank { "optical_blank" };
const char *const kMediaAvailable { "media_available" };
const char *const kCanPowerOff { "can_power_off" };
const char *const kEjectable { "ejectable" };
const char *const kIsEncrypted { "is_encrypted" };
const char *const kIsLoopDevice { "is_loop_device" };
const char *const kHasFileSystem { "has_filesystem" };
const char *const kHasPartitionTable { "has_partition_table" };
const char *const kHasPartition { "has_partition" };
const char *const kHasExtendedPatition { "has_extended_partition" };
const char *const kHintSystem { "hint_system" };
const char *const kHintIgnore { "hint_ignore" };
const char *const kCryptoBackingDevice { "crypto_backingDevice" };
const char *const kDrive { "drive" };
const char *const kMountPoints { "mountpoints" };
const char *const kMediaCompatibility { "media_compatibility" };
const char *const kCleartextDevice { "cleartext_device" };

const char *const kDisplayName { "display_name" };
const char *const kDeviceIcon { "device_icon" };

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
