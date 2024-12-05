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
    if (!tpm.getRandom(size, output))
        return false;

    // Determine whether the password is a hexadecimal character
    QString out = *output;
    int count = out.size();
    if (count != size) {
        qCritical() << "Vault: random password create error! The error password is %1" << out;
        return false;
    }
    for (int i = 0; i < count; ++i) {
        if (!((out[i] >= '0' && out[i] <= '9') || (out[i] >= 'a' && out[i] <= 'f'))) {
            qCritical() << "Vault: random password create error! The error password is %1" << out;
            return false;
        }
    }
    return true;
}

bool EventReceiver::isTpmSupportAlgo(const QString &algoName, bool *support)
{
    if (algoName.isEmpty())
        return false;

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
    if (!encryptParams.contains(PropertyKey::kEncryptType))
        return -1;

    int type = encryptParams.value(PropertyKey::kEncryptType).toInt();
    if (type != 1 && type != 2 && type != 3)
        return -1;

    if (!encryptParams.contains(PropertyKey::kSessionHashAlgo)
        || !encryptParams.contains(PropertyKey::kSessionKeyAlgo)
        || !encryptParams.contains(PropertyKey::kPrimaryHashAlgo)
        || !encryptParams.contains(PropertyKey::kPrimaryKeyAlgo)
        || !encryptParams.contains(PropertyKey::kMinorHashAlgo)
        || !encryptParams.contains(PropertyKey::kMinorKeyAlgo)
        || !encryptParams.contains(PropertyKey::kDirPath)
        || !encryptParams.contains(PropertyKey::kPlain)) {
        return -1;
    }

    if (type == 1) {
        if (!encryptParams.contains(PropertyKey::kPcr)
            || !encryptParams.contains(PropertyKey::kPcrBank)) {
            return -1;
        }
    } else if (type == 2) {
        if (!encryptParams.contains(PropertyKey::kPinCode)) {
            return -1;
        }
    } else if (type == 3) {
        if (!encryptParams.contains(PropertyKey::kPcr)
            || !encryptParams.contains(PropertyKey::kPcrBank)
            || !encryptParams.contains(PropertyKey::kPinCode)) {
            return -1;
        }
    }

    EncryptParams params;
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
    } else if (type == 2) {
        params.type = kTpmAndPin;
        params.pinCode = encryptParams.value(PropertyKey::kPinCode).toString();
    } else if (type == 3) {
        params.type = kTpmAndPcrAndPin;
        params.pcr = encryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = encryptParams.value(PropertyKey::kPcrBank).toString();
        params.pinCode = encryptParams.value(PropertyKey::kPinCode).toString();
    }
    TPMWork tpm;
    return tpm.encryptByTools(params);
}

int EventReceiver::decryptByTpmProcess(const QVariantMap &decryptParams, QString *pwd)
{
    if (!decryptParams.contains(PropertyKey::kEncryptType))
        return -1;

    int type = decryptParams.value(PropertyKey::kEncryptType).toInt();
    if (type != 1 && type != 2 && type != 3)
        return -1;

    if (!decryptParams.contains(PropertyKey::kSessionHashAlgo)
        || !decryptParams.contains(PropertyKey::kSessionKeyAlgo)
        || !decryptParams.contains(PropertyKey::kPrimaryHashAlgo)
        || !decryptParams.contains(PropertyKey::kPrimaryKeyAlgo)
        || !decryptParams.contains(PropertyKey::kDirPath)) {
        return false;
    }

    if (type == 1) {
        if (!decryptParams.contains(PropertyKey::kPcr)
            || !decryptParams.contains(PropertyKey::kPcrBank)) {
            return false;
        }
    } else if (type == 2) {
        if (!decryptParams.contains(PropertyKey::kPinCode)) {
            return false;
        }
    } else if (type == 3) {
        if (!decryptParams.contains(PropertyKey::kPcr)
            || !decryptParams.contains(PropertyKey::kPcrBank)
            || !decryptParams.contains(PropertyKey::kPinCode)) {
            return false;
        }
    }

    DecryptParams params;
    params.sessionHashAlgo = decryptParams.value(PropertyKey::kSessionHashAlgo).toString();
    params.sessionKeyAlgo = decryptParams.value(PropertyKey::kSessionKeyAlgo).toString();
    params.primaryHashAlgo = decryptParams.value(PropertyKey::kPrimaryHashAlgo).toString();
    params.primaryKeyAlgo = decryptParams.value(PropertyKey::kPrimaryKeyAlgo).toString();
    params.dirPath = decryptParams.value(PropertyKey::kDirPath).toString();
    if (type == 1) {
        params.type = kTpmAndPcr;
        params.pcr = decryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = decryptParams.value(PropertyKey::kPcrBank).toString();
    } else if (type == 2) {
        params.type = kTpmAndPin;
        params.pinCode = decryptParams.value(PropertyKey::kPinCode).toString();
    } else if (type == 3) {
        params.type = kTpmAndPcrAndPin;
        params.pcr = decryptParams.value(PropertyKey::kPcr).toString();
        params.pcr_bank = decryptParams.value(PropertyKey::kPcrBank).toString();
        params.pinCode = decryptParams.value(PropertyKey::kPinCode).toString();
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
    initConnection();
}

void EventReceiver::initConnection()
{
    // slot event
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_TPMIsAvailable", this, &EventReceiver::tpmIsAvailable);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_GetRandomByTPM", this, &EventReceiver::getRandomByTpm);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgo", this, &EventReceiver::isTpmSupportAlgo);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_EncryptByTPM", this, &EventReceiver::encrypyByTpm);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_DecryptByTPM", this, &EventReceiver::decryptByTpm);

    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_TPMIsAvailablePro", this, &EventReceiver::tpmIsAvailableProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_GetRandomByTPMPro", this, &EventReceiver::getRandomByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgoPro", this, &EventReceiver::isTpmSupportAlgoProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_EncryptByTPMPro", this, &EventReceiver::encryptByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_DecryptByTPMPro", this, &EventReceiver::decryptByTpmProcess);
    dpfSlotChannel->connect("dfmplugin_encrypt_manager", "slot_OwnerAuthStatus", this, &EventReceiver::ownerAuthStatus);
}
