// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_ACCESSCONTROL_GLOBAL_H
#define DAEMONPLUGIN_ACCESSCONTROL_GLOBAL_H

#define DAEMONPAC_NAMESPACE daemonplugin_accesscontrol

#define DAEMONPAC_BEGIN_NAMESPACE namespace DAEMONPAC_NAMESPACE {
#define DAEMONPAC_END_NAMESPACE }
#define DAEMONPAC_USE_NAMESPACE using namespace DAEMONPAC_NAMESPACE;

DAEMONPAC_BEGIN_NAMESPACE

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

DAEMONPAC_END_NAMESPACE

#endif   // DAEMONPLUGIN_ACCESSCONTROL_GLOBAL_H
