// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_MOUNTCONTROL_GLOBAL_H
#define SERVICE_MOUNTCONTROL_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define SERVICEMOUNTCONTROL_NAMESPACE service_mountcontrol
#define SERVICEMOUNTCONTROL_BEGIN_NAMESPACE namespace SERVICEMOUNTCONTROL_NAMESPACE {
#define SERVICEMOUNTCONTROL_END_NAMESPACE }
#define SERVICEMOUNTCONTROL_USE_NAMESPACE using namespace SERVICEMOUNTCONTROL_NAMESPACE;

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(SERVICEMOUNTCONTROL_NAMESPACE)

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
    kAuthenticationFailed,

    kUnhandledError = 1000
};

SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // SERVICE_MOUNTCONTROL_GLOBAL_H
