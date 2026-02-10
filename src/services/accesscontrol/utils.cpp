// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmInfo() << "[Utils::setFileMode] Changing file mode for path:" << bytes << "to mode:" << QString::number(mode, 8);
    int result = chmod(bytes.data(), mode);
    if (result != 0) {
        fmCritical() << "[Utils::setFileMode] Failed to change file mode for path:" << bytes << "error:" << strerror(errno);
    } else {
        fmInfo() << "[Utils::setFileMode] Successfully changed file mode for path:" << bytes;
    }
    return result;
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

void Utils::saveDevPolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(devConfigPath());
    if (!config.open(QIODevice::ReadWrite)) {
        fmCritical() << "[Utils::saveDevPolicy] Failed to open device policy config file:" << devConfigPath();
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

    fmInfo() << "[Utils::saveDevPolicy] Saving device policy - type:" << inType << "policy:" << inPolicy << "invoker:" << inInvoker << "global:" << inGlobal;

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
                fmInfo() << "[Utils::saveDevPolicy] Found existing policy, updating it for type:" << type;
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
        fmInfo() << "[Utils::saveDevPolicy] Adding new policy for type:" << inType;
    }
    doc.setArray(newArr);
    config.close();
    config.open(QIODevice::Truncate | QIODevice::ReadWrite);   // overwrite the config file
    config.write(doc.toJson());
    config.close();
    fmInfo() << "[Utils::saveDevPolicy] Device policy saved successfully";
}

void Utils::loadDevPolicy(DevPolicyType *devPolicies)
{
    Q_ASSERT(devPolicies);

    QFile config(devConfigPath());
    if (!config.open(QIODevice::ReadOnly)) {
        fmWarning() << "[Utils::loadDevPolicy] Cannot open device policy config file:" << devConfigPath();
        return;
    }
    
    fmInfo() << "[Utils::loadDevPolicy] Loading device policies from:" << devConfigPath();
    
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (err.error != QJsonParseError::NoError) {
        fmCritical() << "[Utils::loadDevPolicy] Failed to parse device policy JSON:" << err.errorString();
        return;
    }

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

    fmInfo() << "[Utils::loadDevPolicy] Loaded device policies:" << *devPolicies;
}

void Utils::saveVaultPolicy(const QVariantMap &policy)
{
    // 1. if file does not exist then create it
    QFile config(valultConfigPath());
    if (!config.open(QIODevice::ReadWrite)) {
        fmCritical() << "[Utils::saveVaultPolicy] Failed to open vault policy config file:" << valultConfigPath();
        config.close();
        return;
    }
    config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    fmInfo() << "[Utils::saveVaultPolicy] Saving vault policy - type:" << policy.value(kPolicyType).toInt() 
             << "hide state:" << policy.value(kVaultHideState).toInt() 
             << "policy state:" << policy.value(kPolicyState).toInt();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    QJsonArray newArr;

    QJsonObject obj;
    obj.insert(kPolicyType, policy.value(kPolicyType).toInt());
    obj.insert(kVaultHideState, policy.value(kVaultHideState).toInt());
    obj.insert(kPolicyState, policy.value(kPolicyState).toInt());
    newArr.append(obj);
    fmInfo() << "[Utils::saveVaultPolicy] Adding new vault policy";
    doc.setArray(newArr);
    config.open(QIODevice::Truncate | QIODevice::ReadWrite);   // overwrite the config file
    config.write(doc.toJson());
    config.close();
    fmInfo() << "[Utils::saveVaultPolicy] Vault policy saved successfully";
}

void Utils::loadVaultPolicy(VaultPolicyType *vaultPolicies)
{
    Q_ASSERT(vaultPolicies);

    QFile config(valultConfigPath());
    if (!config.open(QIODevice::ReadOnly)) {
        fmWarning() << "[Utils::loadVaultPolicy] Cannot open vault policy config file:" << valultConfigPath();
        return;
    }
    
    fmInfo() << "[Utils::loadVaultPolicy] Loading vault policies from:" << valultConfigPath();
    
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(config.readAll(), &err);
    config.close();

    if (err.error != QJsonParseError::NoError) {
        fmCritical() << "[Utils::loadVaultPolicy] Failed to parse vault policy JSON:" << err.errorString();
        return;
    }

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

    fmInfo() << "[Utils::loadVaultPolicy] Loaded vault policies:" << *vaultPolicies;
}

DPCErrorCode Utils::checkDiskPassword(crypt_device **cd, const char *pwd, const char *device)
{
    fmInfo() << "[Utils::checkDiskPassword] Checking disk password for device:" << device;
    
    int r = crypt_init(cd, device);
    if (r < 0) {
        fmCritical() << "[Utils::checkDiskPassword] crypt_init failed for device:" << device << "error code:" << r;
        return kInitFailed;
    }

    r = crypt_load(*cd, /* crypt context */
                   CRYPT_LUKS2, /* requested type */
                   nullptr); /* additional parameters (not used) */

    if (r < 0) {
        fmCritical() << "[Utils::checkDiskPassword] crypt_load failed for device:" << crypt_get_device_name(*cd) << "error code:" << r;
        crypt_free(*cd);
        return kDeviceLoadFailed;
    }

    r = crypt_activate_by_passphrase(*cd, nullptr, CRYPT_ANY_SLOT,
                                     pwd, strlen(pwd), CRYPT_ACTIVATE_ALLOW_UNBOUND_KEY);
    if (r < 0) {
        fmWarning() << "[Utils::checkDiskPassword] Password verification failed for device:" << crypt_get_device_name(*cd) << "error code:" << r;
        crypt_free(*cd);
        return kPasswordWrong;
    }

    fmInfo() << "[Utils::checkDiskPassword] Password verification successful for device:" << crypt_get_device_name(*cd);
    return kNoError;
}

DPCErrorCode Utils::changeDiskPassword(crypt_device *cd, const char *oldPwd, const char *newPwd)
{
    Q_ASSERT(cd);

    const char *deviceName = crypt_get_device_name(cd);
    fmInfo() << "[Utils::changeDiskPassword] Changing password for device:" << deviceName;

    int r = crypt_keyslot_change_by_passphrase(cd, CRYPT_ANY_SLOT,
                                               CRYPT_ANY_SLOT, oldPwd,
                                               strlen(oldPwd), newPwd, strlen(newPwd));
    crypt_free(cd);
    if (r < 0) {
        fmCritical() << "[Utils::changeDiskPassword] Failed to change password for device:" << deviceName << "error code:" << r;
        return kPasswordChangeFailed;
    }

    fmInfo() << "[Utils::changeDiskPassword] Password changed successfully for device:" << deviceName;
    return kNoError;
}
