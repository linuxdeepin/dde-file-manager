// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "tpm/tpmwork.h"

#include <dfm-framework/event/event.h>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

DPENCRYPTMANAGER_USE_NAMESPACE

EventReceiver *EventReceiver::instance()
{
    static EventReceiver ins;
    return &ins;
}

bool EventReceiver::tpmIsAvailable()
{
    TPMWork tpm;
    return tpm.checkTPMAvailable();
}

bool EventReceiver::getRandomByTpm(int size, QString *output)
{
    TPMWork tpm;
    if (!tpm.getRandom(size, output)) {
        fmWarning() << "Failed to generate random data by TPM";
        return false;
    }

    // Determine whether the password is a hexadecimal character
    QString out = *output;
    int count = out.size();
    if (count != size) {
        qCritical() << "Vault: random password create error! The error password is %1" << out;
        fmCritical() << "Random password size mismatch - expected:" << size << "actual:" << count;
        return false;
    }

    for (int i = 0; i < count; ++i) {
        if (!((out[i] >= '0' && out[i] <= '9') || (out[i] >= 'a' && out[i] <= 'f'))) {
            qCritical() << "Vault: random password create error! The error password is %1" << out;
            fmCritical() << "Random password contains invalid characters at position:" << i;
            return false;
        }
    }
    return true;
}

bool EventReceiver::isTpmSupportAlgo(const QString &algoName, bool *support)
{
    if (algoName.isEmpty()) {
        fmWarning() << "Empty algorithm name provided for TPM support check";
        return false;
    }

    TPMWork tpm;
    return tpm.isSupportAlgo(algoName, support);
}

bool EventReceiver::encrypyByTpm(const QString &hashAlgo, const QString &keyAlgo, const QString &keyPin, const QString &password, const QString &dirPath)
{
    TPMWork tpm;
    return tpm.encrypt(hashAlgo, keyAlgo, keyPin, password, dirPath);
}

bool EventReceiver::decryptByTpm(const QString &keyPin, const QString &dirPath, QString *pwd)
{
    TPMWork tpm;
    return tpm.decrypt(keyPin, dirPath, pwd);
}

int EventReceiver::tpmIsAvailableProcess()
{
    TPMWork tpm;
    return tpm.checkTPMAvailbableByTools();
}

int EventReceiver::tpmCheckLockoutStatusProcess()
{
    return TPMWork::checkTPMLockoutStatusByTools();
}

int EventReceiver::getRandomByTpmProcess(int size, QString *output)
{
    TPMWork tpm;
    return tpm.getRandomByTools(size, output);
}

int EventReceiver::isTpmSupportAlgoProcess(const QString &algoName, bool *support)
{
    TPMWork tpm;
    return tpm.isSupportAlgoByTools(algoName, support);
}

int EventReceiver::encryptByTpmProcess(const QVariantMap &encryptParams)
{
    if (!encryptParams.contains(PropertyKey::kEncryptType)) {
        fmWarning() << "Missing encrypt type in parameters";
        return -1;
    }

    int type = encryptParams.value(PropertyKey::kEncryptType).toInt();
    if (type != 1 && type != 2 && type != 3) {
        fmWarning() << "Invalid encrypt type:" << type;
        return -1;
    }

    if (!encryptParams.contains(PropertyKey::kSessionHashAlgo)
        || !encryptParams.contains(PropertyKey::kSessionKeyAlgo)
        || !encryptParams.contains(PropertyKey::kPrimaryHashAlgo)
        || !encryptParams.contains(PropertyKey::kPrimaryKeyAlgo)
        || !encryptParams.contains(PropertyKey::kMinorHashAlgo)
        || !encryptParams.contains(PropertyKey::kMinorKeyAlgo)
        || !encryptParams.contains(PropertyKey::kDirPath)
        || !encryptParams.contains(PropertyKey::kPlain)) {
        fmWarning() << "Missing required encryption parameters";
        return -1;
    }

    if (type == 1) {
        if (!encryptParams.contains(PropertyKey::kPcr)
            || !encryptParams.contains(PropertyKey::kPcrBank)) {
            fmWarning() << "Missing PCR parameters for type 1 encryption";
            return -1;
        }
    } else if (type == 2) {
        if (!encryptParams.contains(PropertyKey::kPinCode)) {
            fmWarning() << "Missing PIN code for type 2 encryption";
            return -1;
        }
    } else if (type == 3) {
        if (!encryptParams.contains(PropertyKey::kPcr)
            || !encryptParams.contains(PropertyKey::kPcrBank)
            || !encryptParams.contains(PropertyKey::kPinCode)) {
            fmWarning() << "Missing PCR or PIN parameters for type 3 encryption";
            return -1;
        }
    }

    TpmEncryptArgs params;
    params.sessionHashAlgo = encryptParams.value(PropertyKey::kSessionHashAlgo).toString();
    params.sessionKeyAlgo = encryptParams.value(PropertyKey::kSessionKeyAlgo).toString();
    params.primaryHashAlgo = encryptParams.value(PropertyKey::kPrimaryHashAlgo).toString();
    params.primaryKeyAlgo = encryptParams.value(PropertyKey::kPrimaryKeyAlgo).toString();
    params.minorHashAlgo = encryptParams.value(PropertyKey::kMinorHashAlgo).toString();
    params.minorKeyAlgo = encryptParams.value(PropertyKey::kMinorKeyAlgo).toString();
    params.dirPath = encryptParams.value(PropertyKey::kDirPath).toString();
    params.plain = encryptParams.value(PropertyKey::kPlain).toString();
    if (type == 1) {
        params.type = kTpmAndPcr;
        params.pcr = encryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = encryptParams.value(PropertyKey::kPcrBank).toString();
        fmDebug() << "Type 1 encryption (TPM+PCR) - PCR:" << params.pcr << "PCR Bank:" << params.pcr_bank;
    } else if (type == 2) {
        params.type = kTpmAndPin;
        params.pinCode = encryptParams.value(PropertyKey::kPinCode).toString();
        fmDebug() << "Type 2 encryption (TPM+PIN)";
    } else if (type == 3) {
        params.type = kTpmAndPcrAndPin;
        params.pcr = encryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = encryptParams.value(PropertyKey::kPcrBank).toString();
        params.pinCode = encryptParams.value(PropertyKey::kPinCode).toString();
        fmDebug() << "Type 3 encryption (TPM+PCR+PIN) - PCR:" << params.pcr << "PCR Bank:" << params.pcr_bank;
    }

    TPMWork tpm;
    return tpm.encryptByTools(params);
}

int EventReceiver::decryptByTpmProcess(const QVariantMap &decryptParams, QString *pwd)
{
    if (!decryptParams.contains(PropertyKey::kEncryptType)) {
        fmWarning() << "Missing encrypt type in decryption parameters";
        return -1;
    }

    int type = decryptParams.value(PropertyKey::kEncryptType).toInt();
    if (type != 1 && type != 2 && type != 3) {
        fmWarning() << "Invalid decrypt type:" << type;
        return -1;
    }

    if (!decryptParams.contains(PropertyKey::kSessionHashAlgo)
        || !decryptParams.contains(PropertyKey::kSessionKeyAlgo)
        || !decryptParams.contains(PropertyKey::kPrimaryHashAlgo)
        || !decryptParams.contains(PropertyKey::kPrimaryKeyAlgo)
        || !decryptParams.contains(PropertyKey::kDirPath)) {
        fmWarning() << "Missing required decryption parameters";
        return false;
    }

    if (type == 1) {
        if (!decryptParams.contains(PropertyKey::kPcr)
            || !decryptParams.contains(PropertyKey::kPcrBank)) {
            fmWarning() << "Missing PCR parameters for type 1 decryption";
            return false;
        }
    } else if (type == 2) {
        if (!decryptParams.contains(PropertyKey::kPinCode)) {
            fmWarning() << "Missing PIN code for type 2 decryption";
            return false;
        }
    } else if (type == 3) {
        if (!decryptParams.contains(PropertyKey::kPcr)
            || !decryptParams.contains(PropertyKey::kPcrBank)
            || !decryptParams.contains(PropertyKey::kPinCode)) {
            fmWarning() << "Missing PCR or PIN parameters for type 3 decryption";
            return false;
        }
    }

    TpmDecryptArgs params;
    params.sessionHashAlgo = decryptParams.value(PropertyKey::kSessionHashAlgo).toString();
    params.sessionKeyAlgo = decryptParams.value(PropertyKey::kSessionKeyAlgo).toString();
    params.primaryHashAlgo = decryptParams.value(PropertyKey::kPrimaryHashAlgo).toString();
    params.primaryKeyAlgo = decryptParams.value(PropertyKey::kPrimaryKeyAlgo).toString();
    params.dirPath = decryptParams.value(PropertyKey::kDirPath).toString();
    if (type == 1) {
        params.type = kTpmAndPcr;
        params.pcr = decryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = decryptParams.value(PropertyKey::kPcrBank).toString();
        fmDebug() << "Type 1 decryption (TPM+PCR) - PCR:" << params.pcr << "PCR Bank:" << params.pcr_bank;
    } else if (type == 2) {
        params.type = kTpmAndPin;
        params.pinCode = decryptParams.value(PropertyKey::kPinCode).toString();
        fmDebug() << "Type 2 decryption (TPM+PIN)";
    } else if (type == 3) {
        params.type = kTpmAndPcrAndPin;
        params.pcr = decryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = decryptParams.value(PropertyKey::kPcrBank).toString();
        params.pinCode = decryptParams.value(PropertyKey::kPinCode).toString();
        fmDebug() << "Type 3 decryption (TPM+PCR+PIN) - PCR:" << params.pcr << "PCR Bank:" << params.pcr_bank;
    }

    TPMWork tpm;
    return tpm.decryptByTools(params, pwd);
}

int EventReceiver::ownerAuthStatus()
{
    return TPMWork().ownerAuthStatus();
}

EventReceiver::EventReceiver(QObject *parent)
    : QObject(parent)
{
    fmDebug() << "EventReceiver constructed";
    initConnection();
}

void EventReceiver::initConnection()
{
    fmDebug() << "Initializing EventReceiver connections";

    // slot event
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_TPMIsAvailable", this, &EventReceiver::tpmIsAvailable);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_GetRandomByTPM", this, &EventReceiver::getRandomByTpm);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgo", this, &EventReceiver::isTpmSupportAlgo);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_EncryptByTPM", this, &EventReceiver::encrypyByTpm);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_DecryptByTPM", this, &EventReceiver::decryptByTpm);

    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_TPMIsAvailablePro", this, &EventReceiver::tpmIsAvailableProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_CheckTPMLockoutPro", this, &EventReceiver::tpmCheckLockoutStatusProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_GetRandomByTPMPro", this, &EventReceiver::getRandomByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgoPro", this, &EventReceiver::isTpmSupportAlgoProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_EncryptByTPMPro", this, &EventReceiver::encryptByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_DecryptByTPMPro", this, &EventReceiver::decryptByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_OwnerAuthStatus", this, &EventReceiver::ownerAuthStatus);

    fmDebug() << "EventReceiver slot connections established - 12 slots connected";
}
