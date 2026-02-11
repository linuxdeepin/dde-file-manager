// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_ACCESSCONTROL_GLOBAL_H
#define SERVICE_ACCESSCONTROL_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define SERVICEACCESSCONTROL_NAMESPACE service_accesscontrol

#define SERVICEACCESSCONTROL_BEGIN_NAMESPACE namespace SERVICEACCESSCONTROL_NAMESPACE {
#define SERVICEACCESSCONTROL_END_NAMESPACE }
#define SERVICEACCESSCONTROL_USE_NAMESPACE using namespace SERVICEACCESSCONTROL_NAMESPACE;

SERVICEACCESSCONTROL_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(SERVICEACCESSCONTROL_NAMESPACE)

// error code of change disk password
enum DPCErrorCode {
    kNoError = 0,
    kAuthenticationFailed,
    kInitFailed,
    kDeviceLoadFailed,
    kPasswordChangeFailed,
    kPasswordWrong,
    kAccessDiskFailed,   // Unable to get the encrypted disk list
    kPasswordInconsistent   // Passwords of disks are different
};

SERVICEACCESSCONTROL_END_NAMESPACE

#endif   // SERVICE_ACCESSCONTROL_GLOBAL_H
