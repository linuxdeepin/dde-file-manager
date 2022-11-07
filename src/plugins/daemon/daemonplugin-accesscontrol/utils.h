/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef UTILS_H
#define UTILS_H

#include "daemonplugin_accesscontrol_global.h"

#include <QObject>

struct crypt_device;

DAEMONPAC_BEGIN_NAMESPACE

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
    static const QStringList whiteProcess();

    static const QString devConfigPath();
    static const QString valultConfigPath();
    static int accessMode(const QString &mps);
    static void addWriteMode(const QString &mountPoint);

    static bool isValidDevPolicy(const QVariantMap &policy, const QString &realInvoker);
    static bool isValidVaultPolicy(const QVariantMap &policy);
    static bool isValidInvoker(uint pid, QString &invokerPath);

    static void saveDevPolicy(const QVariantMap &policy);
    static void loadDevPolicy(DevPolicyType *devPolicies);
    static void saveVaultPolicy(const QVariantMap &policy);
    static void loadVaultPolicy(VaultPolicyType *vaultPolicies);

    static DPCErrorCode checkDiskPassword(crypt_device *cd, const char *pwd, const char *device);
    static DPCErrorCode changeDiskPassword(crypt_device *cd, const char *oldPwd, const char *newPwd);
};

DAEMONPAC_END_NAMESPACE

#endif   // UTILS_H
