// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H
#define DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H

#define DAEMONPMOUNTCONTROL_NAMESPACE daemonplugin_mountcontrol
#define DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE namespace DAEMONPMOUNTCONTROL_NAMESPACE {
#define DAEMONPMOUNTCONTROL_END_NAMESPACE }
#define DAEMONPMOUNTCONTROL_USE_NAMESPACE using namespace DAEMONPMOUNTCONTROL_NAMESPACE;

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE

namespace MountOptionsField {
inline constexpr char kUser[] { "user" };
inline constexpr char kPasswd[] { "passwd" };
inline constexpr char kDomain[] { "domain" };
inline constexpr char kFsType[] { "fsType" };
inline constexpr char kPort[] { "port" };
inline constexpr char kIp[] { "ip" };
inline constexpr char kVersion[] { "version" };
inline constexpr char kMountName[] { "mntName" };
inline constexpr char kTimeout[] { "timeout" };
inline constexpr char kTryWaitReconn[] { "waitReconn" };
inline constexpr char kUnmountAllStacked[] { "unmountAllStacked" };
}

namespace MountReturnField {
inline constexpr char kResult[] { "result" };
inline constexpr char kMountPoint[] { "mountPoint" };
inline constexpr char kErrorCode[] { "errno" };
inline constexpr char kErrorMessage[] { "errMsg" };
}

namespace MountFstypeSupportedField {
inline constexpr char kDlnFs[] { "dlnfs" };
inline constexpr char kCifs[] { "cifs" };
inline constexpr char kCommon[] { "common" };
}

enum MountErrorCode {   // NOTE(xust): use NAGATIVE value when return, avoid confusing with errno
    kNotSupportedScheme = 1,
    kCannotGenerateMountPath,
    kCannotMkdirMountPoint,
    kNoFsTypeSpecified,
    kUnsupportedFsTypeOrProtocol,
    kMountNotExist,
    kNotOwnerOfMount,

    kDlnFsProcessNotExists,
    kDlnMountMounted,
    kFusermountProcessNotExists,

    kUnhandledError = 1000
};

DAEMONPMOUNTCONTROL_END_NAMESPACE

#endif   // DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H
