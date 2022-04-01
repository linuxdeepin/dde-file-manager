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

DAEMONPAC_BEGIN_NAMESPACE

static constexpr char kKeyInvoker[] { "invoker" };
static constexpr char kKeyType[] { "type" };
static constexpr char kKeyDevice[] { "device" };
static constexpr char kKeyPolicy[] { "policy" };
static constexpr char kKeyTstamp[] { "timestamp" };
static constexpr char kKeyGlobal[] { "global" };

static constexpr char kKeyErrstr[] { "errstr" };
static constexpr char kKeyErrno[] { "errno" };

static constexpr char kPolicyType[] { "policytype" };
static constexpr char kVaultHideState[] { "vaulthidestate" };
static constexpr char kPolicyState[] { "policystate" };

static constexpr int kTypeInvalid { 0x00 };
static constexpr int kTypeBlock { 0x01 };
static constexpr int kTypeOptical { 0x02 };
static constexpr int kTypeProtocol { 0x04 };

static constexpr int kPolicyDisable { 0 };
static constexpr int kPolicyRonly { 1 };
static constexpr int kPolicyRw { 2 };

static constexpr int kMaxRetry { 5 };

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
};

DAEMONPAC_END_NAMESPACE

#endif   // UTILS_H
