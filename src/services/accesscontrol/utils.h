// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include "service_accesscontrol_global.h"

#include <QObject>

struct crypt_device;

SERVICEACCESSCONTROL_BEGIN_NAMESPACE

inline constexpr char kKeyInvoker[] { "invoker" };
inline constexpr char kKeyType[] { "type" };
inline constexpr char kKeyDevice[] { "device" };
inline constexpr char kKeyPolicy[] { "policy" };
inline constexpr char kKeyTstamp[] { "timestamp" };
inline constexpr char kKeyGlobal[] { "global" };

inline constexpr char kKeyErrstr[] { "errstr" };
inline constexpr char kKeyErrno[] { "errno" };

inline constexpr char kPolicyType[] { "policytype" };
inline constexpr char kVaultHideState[] { "vaulthidestate" };
inline constexpr char kPolicyState[] { "policystate" };

inline constexpr int kTypeInvalid { 0x00 };
inline constexpr int kTypeBlock { 0x01 };
inline constexpr int kTypeOptical { 0x02 };
inline constexpr int kTypeProtocol { 0x04 };

inline constexpr int kPolicyDisable { 0 };
inline constexpr int kPolicyRonly { 1 };
inline constexpr int kPolicyRw { 2 };

inline constexpr int kMaxRetry { 5 };

using DevPolicyType = QMap<int, QPair<QString, int>>;
using VaultPolicyType = QMap<QString, int>;

class Utils
{
public:
    static const QString devConfigPath();
    static const QString valultConfigPath();
    static int accessMode(const QString &mps);
    static int setFileMode(const QString &mountPoint, uint mode);

    static bool isValidDevPolicy(const QVariantMap &policy, const QString &realInvoker);
    static bool isValidVaultPolicy(const QVariantMap &policy);

    static void saveDevPolicy(const QVariantMap &policy);
    static void loadDevPolicy(DevPolicyType *devPolicies);
    static void saveVaultPolicy(const QVariantMap &policy);
    static void loadVaultPolicy(VaultPolicyType *vaultPolicies);

    static DPCErrorCode checkDiskPassword(crypt_device **cd, const char *pwd, const char *device);
    static DPCErrorCode changeDiskPassword(crypt_device *cd, const char *oldPwd, const char *newPwd);
};

SERVICEACCESSCONTROL_END_NAMESPACE

#endif   // UTILS_H
