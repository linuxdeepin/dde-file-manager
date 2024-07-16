// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QVariantMap>
#include <QFileInfo>
#include <QDateTime>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libcryptsetup.h>

SERVICEACCESSCONTROL_USE_NAMESPACE

const QStringList Utils::whiteProcess()
{
    // TODO(xust) add python3.7 so that we can invoke the method through d-feet.
    static const QStringList processList { "/usr/bin/dmcg", "/usr/bin/dde-file-manager", "/usr/bin/python3.7" };
    return processList;
}

const QString Utils::devConfigPath()
{
    static const QString path { "/etc/deepin/devAccessConfig.json" };
    return path;
}

const QString Utils::valultConfigPath()
{
    static const QString path { "/etc/deepin/vaultAccessConfig.json" };
    return path;
}

int Utils::accessMode(const QString &mps)
{
    // 获取挂载点访问权限
    if (mps.isEmpty())
        return kPolicyDisable;
    const QByteArray &path = mps.toLocal8Bit();
    if (access(path.data(), W_OK) == 0)
        return kPolicyRw;
    if (access(path.data(), R_OK) == 0)
        return kPolicyRonly;
    return kPolicyDisable;
}

int Utils::setFileMode(const QString &mountPoint, uint mode)
{
    QByteArray bytes { mountPoint.toLocal8Bit() };
    fmInfo() << "chmod ==>" << bytes << "to" << mode;
    return chmod(bytes.data(), mode);
}

bool Utils::isValidDevPolicy(const QVariantMap &policy, const QString &realInvoker)
{
    // invoker must not be empty
    // type must in (0, 7]
    // policy must in [0, 2]
    // device is optional
    return policy.contains(kKeyInvoker) && !policy.value(kKeyInvoker).toString().isEmpty()
            && policy.contains(kKeyType) && policy.value(kKeyType).toInt() > kTypeInvalid && policy.value(kKeyType).toInt() <= (kTypeBlock | kTypeOptical | kTypeProtocol)
            && policy.contains(kKeyPolicy) && policy.value(kKeyPolicy).toInt() >= kPolicyDisable && policy.value(kKeyPolicy).toInt() <= kPolicyRw
            && policy.value(kKeyInvoker).toString() == realInvoker;
}

bool Utils::isValidVaultPolicy(const QVariantMap &policy)
{
    if (policy.value(kPolicyType).toInt() < 0 || policy.value(kVaultHideState).toInt() < 0)
        return false;
    return true;
}

bool Utils::isValidInvoker(uint pid, QString &invokerPath)
{
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    if (!f.exists())
        return false;
    invokerPath = f.canonicalFilePath();
    return whiteProcess().contains(invokerPath);
}

void Utils::saveDevPolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(devConfigPath());
    if (!config.open(QIODevice::ReadWrite)) {
        fmDebug() << "config open failed";
        return;
    }
    config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    int inGlobal = (!policy.contains(kKeyDevice) || policy.value(kKeyDevice).toString().isEmpty())
            ? 1
            : 0;
    int inType = policy.value(kKeyType).toInt();
    int inPolicy = policy.value(kKeyPolicy).toInt();
    QString inDevice = inGlobal ? "" : policy.value(kKeyDevice).toString();
    QString inInvoker = policy.value(kKeyInvoker).toString();

    // 2. append/replace config to configFile
    bool foundExist = false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    QJsonArray newArr;
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (auto obj : arr) {
            if (!obj.isObject())
                continue;
            QJsonObject objInfo = obj.toObject();
            int global = objInfo.contains(kKeyGlobal) ? objInfo.value(kKeyGlobal).toInt() : 0;
            QString src = objInfo.contains(kKeyInvoker) ? objInfo.value(kKeyInvoker).toString() : "";
            int type = objInfo.contains(kKeyType) ? objInfo.value(kKeyType).toInt() : 0;
            QString timestamp = objInfo.contains(kKeyTstamp) ? objInfo.value(kKeyTstamp).toString() : "";
            QString dev = objInfo.contains(kKeyDevice) ? objInfo.value(kKeyDevice).toString() : "";
            QString invoker = objInfo.contains(kKeyInvoker) ? objInfo.value(kKeyInvoker).toString() : "";

            if (inGlobal == global && inType == type && inDevice == dev && inInvoker == invoker) {
                foundExist = true;
                objInfo.insert(kKeyPolicy, inPolicy);
                objInfo.insert(kKeyTstamp, QString::number(QDateTime::currentSecsSinceEpoch()));
                fmDebug() << "found exist policy, just updtae it";
            }

            newArr.append(objInfo);
        }
    }
    if (!foundExist) {
        QJsonObject obj;
        obj.insert(kKeyGlobal, inGlobal ? 1 : 0);
        obj.insert(kKeyInvoker, inInvoker);
        obj.insert(kKeyType, inType);
        obj.insert(kKeyPolicy, inPolicy);
        obj.insert(kKeyTstamp, QString::number(QDateTime::currentSecsSinceEpoch()));
        if (inGlobal == 0)
            obj.insert(kKeyDevice, inDevice);
        newArr.append(obj);
        fmDebug() << "append new policy";
    }
    doc.setArray(newArr);
    config.close();
    config.open(QIODevice::Truncate | QIODevice::ReadWrite);   // overwrite the config file
    config.write(doc.toJson());
    config.close();
}

void Utils::loadDevPolicy(DevPolicyType *devPolicies)
{
    Q_ASSERT(devPolicies);

    QFile config(devConfigPath());
    if (!config.open(QIODevice::ReadOnly))
        return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (doc.isArray()) {
        devPolicies->clear();
        QJsonArray arr = doc.array();
        foreach (auto item, arr) {
            if (!item.isObject())
                continue;

            QJsonObject obj = item.toObject();
            int global = obj.contains(kKeyGlobal) ? obj.value(kKeyGlobal).toInt() : -1;
            if (global != 1) {
                // load black/white policy
                ;
            } else {
                // load default/general policy
                int type = obj.contains(kKeyType) ? obj.value(kKeyType).toInt() : kTypeInvalid;
                int policy = obj.contains(kKeyPolicy) ? obj.value(kKeyPolicy).toInt() : kPolicyRw;
                QString source = obj.contains(kKeyInvoker) ? obj.value(kKeyInvoker).toString() : "";
                if (!devPolicies->contains(type) || 1)   // 1: 新读取到的数据源优先级大于之前读取到的，则刷新原有的
                    devPolicies->insert(type, QPair<QString, int>(source, policy));
            }
        }
    }

    fmDebug() << "loaded policy: " << *devPolicies;
}

void Utils::saveVaultPolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(valultConfigPath());
    if (!config.open(QIODevice::ReadWrite)) {
        fmDebug() << "config open failed";
        config.close();
        return;
    }
    config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    QJsonArray newArr;

    QJsonObject obj;
    obj.insert(kPolicyType, policy.value(kPolicyType).toInt());
    obj.insert(kVaultHideState, policy.value(kVaultHideState).toInt());
    obj.insert(kPolicyState, policy.value(kPolicyState).toInt());
    newArr.append(obj);
    fmDebug() << "append new policy";
    doc.setArray(newArr);
    config.open(QIODevice::Truncate | QIODevice::ReadWrite);   // overwrite the config file
    config.write(doc.toJson());
    config.close();
}

void Utils::loadVaultPolicy(VaultPolicyType *vaultPolicies)
{
    Q_ASSERT(vaultPolicies);

    QFile config(valultConfigPath());
    if (!config.open(QIODevice::ReadOnly))
        return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (doc.isArray()) {
        vaultPolicies->clear();
        QJsonArray &&arr = doc.array();
        for (auto item : arr) {
            if (!item.isObject())
                continue;

            QJsonObject obj = item.toObject();

            // load default/general policy
            int policytype = obj.contains(kPolicyType) ? obj.value(kPolicyType).toInt() : -1;
            int vaulthidestate = obj.contains(kVaultHideState) ? obj.value(kVaultHideState).toInt() : -1;
            int policystate = obj.contains(kPolicyState) ? obj.value(kPolicyState).toInt() : -1;
            vaultPolicies->insert(kPolicyType, policytype);
            vaultPolicies->insert(kVaultHideState, vaulthidestate);
            vaultPolicies->insert(kPolicyState, policystate);
        }
    }

    fmDebug() << "loaded policy: " << *vaultPolicies;
}

DPCErrorCode Utils::checkDiskPassword(crypt_device **cd, const char *pwd, const char *device)
{
    int r = crypt_init(cd, device);
    if (r < 0) {
        fmInfo("crypt_init failed,code is:%d", r);
        return kInitFailed;
    }

    r = crypt_load(*cd, /* crypt context */
                   CRYPT_LUKS2, /* requested type */
                   nullptr); /* additional parameters (not used) */

    if (r < 0) {
        fmInfo("crypt_load failed on device %s.\n", crypt_get_device_name(*cd));
        crypt_free(*cd);
        return kDeviceLoadFailed;
    }

    r = crypt_activate_by_passphrase(*cd, nullptr, CRYPT_ANY_SLOT,
                                     pwd, strlen(pwd), CRYPT_ACTIVATE_ALLOW_UNBOUND_KEY);
    if (r < 0) {
        fmInfo("crypt_activate_by_passphrase failed on device %s.\n", crypt_get_device_name(*cd));
        crypt_free(*cd);
        return kPasswordWrong;
    }

    return kNoError;
}

DPCErrorCode Utils::changeDiskPassword(crypt_device *cd, const char *oldPwd, const char *newPwd)
{
    Q_ASSERT(cd);

    int r = crypt_keyslot_change_by_passphrase(cd, CRYPT_ANY_SLOT,
                                               CRYPT_ANY_SLOT, oldPwd,
                                               strlen(oldPwd), newPwd, strlen(newPwd));
    crypt_free(cd);
    if (r < 0) {
        fmInfo("crypt_keyslot_change_by_passphrase failed,code is:%d", r);
        return kPasswordChangeFailed;
    }

    return kNoError;
}
