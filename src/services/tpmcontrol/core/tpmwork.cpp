// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tpmwork.h"

#include <QLibrary>
#include <QDebug>
#include <QFile>
#include <QDir>

// C structures for libutpm2.so
typedef enum {
    kCTpmAndPcr,
    kCTpmAndPin,
    kCTpmAndPcrAndPin
} TpmProType;

typedef struct
{
    TpmProType type;
    char *sessionHashAlgo;
    char *sessionKeyAlgo;
    char *primaryHashAlgo;
    char *primaryKeyAlgo;
    char *minorHashAlgo;
    char *minorKeyAlgo;
    char *dirPath;
    char *plain;
    char *pinCode;
    char *pcr;
    char *pcr_bank;
} Utpm2EncryptParamsByTools;

typedef struct
{
    TpmProType type;
    char *sessionHashAlgo;
    char *sessionKeyAlgo;
    char *primaryHashAlgo;
    char *primaryKeyAlgo;
    char *dirPath;
    char *pinCode;
    char *pcr;
    char *pcr_bank;
} Utpm2DecryptParamsByTools;

inline constexpr char kTpmLibName[] { "libutpm2.so" };

SERVICETPMCONTROL_USE_NAMESPACE

TPMWork::TPMWork(QObject *parent)
    : QObject(parent), tpmLib(new QLibrary(kTpmLibName))
{
    if (!tpmLib->load())
        fmWarning() << "TPMControl: load libutpm2 failed, error:" << tpmLib->errorString();
    else
        fmInfo() << "TPMControl: libutpm2 loaded successfully";
}

TPMWork::~TPMWork()
{
    fmDebug() << "Destroying TPMWork instance";
    if (tpmLib) {
        tpmLib->unload();
        delete tpmLib;
        tpmLib = nullptr;
        fmDebug() << "TPM library unloaded and cleaned up";
    }
}

bool TPMWork::isLibraryLoaded() const
{
    return tpmLib && tpmLib->isLoaded();
}

int TPMWork::isTPMAvailable()
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*p_utpm2_check_tpm_by_tools)(void);
    p_utpm2_check_tpm_by_tools func = (p_utpm2_check_tpm_by_tools)tpmLib->resolve("utpm2_check_tpm_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_check_tpm_by_tools failed!";
        return kLibLoadFailed;
    }

    return func();
}

int TPMWork::checkTPMLockout()
{
    const char* tempFile = "dde_tpmcontrol_lockout_output.txt";
    std::string command = "tpm2_getcap properties-variable > ";
    command += tempFile;
    int ret = system(command.c_str());
    if (ret != 0) {
        remove(tempFile);
        fmCritical() << "Exec" << command << "failed with return code:" << ret;
        return -1;
    }

    FILE* fp = fopen(tempFile, "r");
    if (!fp) {
        remove(tempFile);
        fmCritical() << "Open" << tempFile << "failed!";
        return -1;
    }

    char line[256];
    int lockoutStatus = -2; // not find "inLockout:"
    fmDebug() << "Parsing TPM properties from file:" << tempFile;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "inLockout:")) {
            char* colon = strchr(line, ':');
            if (colon) {
                char* value = colon + 1;
                while (*value && (*value == ' ' || *value == '\t')) {
                    value++;
                }
                int num;
                if (sscanf(value, "%d", &num) == 1) {
                    lockoutStatus = num;
                }
            }
            break;
        }
    }

    if (lockoutStatus == -2)
        fmCritical() << "Not find inLockout in TPM properties";

    fclose(fp);
    remove(tempFile);
    return lockoutStatus;
}

int TPMWork::getRandom(int size, QString *output)
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*p_utpm2_get_random_by_tools)(int size, char *buf);
    p_utpm2_get_random_by_tools func = (p_utpm2_get_random_by_tools)tpmLib->resolve("utpm2_get_random_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_get_random_by_tools failed!";
        return kLibLoadFailed;
    }

    // Validate size parameter to prevent buffer overflow
    if (size <= 0 || size > 4096) {
        fmWarning() << "Invalid random size requested:" << size;
        return kInvalidParam;
    }

    // Dynamically allocate buffer matching the requested size
    QByteArray buffer(size + 1, 0);  // +1 for null terminator
    int ret = func(size, buffer.data());
    if (ret == 0) {
        *output = QString::fromLatin1(buffer.constData());
    }
    return ret;
}

int TPMWork::isSupportAlgo(const QString &algoName, bool *support)
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*p_utpm2_check_alg_by_tools)(const char *algo_name, bool *support);
    p_utpm2_check_alg_by_tools func = (p_utpm2_check_alg_by_tools)tpmLib->resolve("utpm2_check_alg_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_check_alg_by_tools failed!";
        return kLibLoadFailed;
    }

    QByteArray algo = algoName.toUtf8();
    return func(algo.data(), support);
}

int TPMWork::encrypt(const QVariantMap &params)
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*utpm2_encrypt_by_tools)(const Utpm2EncryptParamsByTools *par);
    utpm2_encrypt_by_tools func = (utpm2_encrypt_by_tools)tpmLib->resolve("utpm2_encrypt_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_encrypt_by_tools failed!";
        return kLibLoadFailed;
    }

    // Extract parameters from QVariantMap
    int type = params.value(PropertyKey::kEncryptType).toInt();

    Utpm2EncryptParamsByTools pa;
    if (type == kTpmAndPcr) {
        pa.type = kCTpmAndPcr;
    } else if (type == kTpmAndPin) {
        pa.type = kCTpmAndPin;
    } else if (type == kTpmAndPcrAndPin) {
        pa.type = kCTpmAndPcrAndPin;
    } else {
        fmWarning() << "Invalid encryption type:" << type;
        return kInvalidParam;
    }

    QByteArray arrSessionHashAlgo = params.value(PropertyKey::kSessionHashAlgo).toString().toUtf8();
    pa.sessionHashAlgo = arrSessionHashAlgo.data();
    QByteArray arrSessionKeyAlgo = params.value(PropertyKey::kSessionKeyAlgo).toString().toUtf8();
    pa.sessionKeyAlgo = arrSessionKeyAlgo.data();
    QByteArray arrPriHashAlgo = params.value(PropertyKey::kPrimaryHashAlgo).toString().toUtf8();
    pa.primaryHashAlgo = arrPriHashAlgo.data();
    QByteArray arrPriKeyAlgo = params.value(PropertyKey::kPrimaryKeyAlgo).toString().toUtf8();
    pa.primaryKeyAlgo = arrPriKeyAlgo.data();
    QByteArray arrMinHashAlgo = params.value(PropertyKey::kMinorHashAlgo).toString().toUtf8();
    pa.minorHashAlgo = arrMinHashAlgo.data();
    QByteArray arrMinKeyAlgo = params.value(PropertyKey::kMinorKeyAlgo).toString().toUtf8();
    pa.minorKeyAlgo = arrMinKeyAlgo.data();

    QByteArray arrDirPath = params.value(PropertyKey::kDirPath).toString().toUtf8();
    pa.dirPath = arrDirPath.data();
    QByteArray arrPlain = params.value(PropertyKey::kPlain).toString().toUtf8();
    pa.plain = arrPlain.data();

    QByteArray arrPinCode = params.value(PropertyKey::kPinCode).toString().toUtf8();
    pa.pinCode = arrPinCode.data();

    QByteArray arrPcr = params.value(PropertyKey::kPcr).toString().toUtf8();
    pa.pcr = arrPcr.data();
    QByteArray arrPcrBank = params.value(PropertyKey::kPcrBank).toString().toUtf8();
    pa.pcr_bank = arrPcrBank.data();

    int ret = func(&pa);
    if (ret != 0) {
        fmCritical() << "utpm2_encrypt_by_tools failed with error code:" << ret;
    }

    return ret;
}

int TPMWork::decrypt(const QVariantMap &params, QString *pwd)
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*utpm2_decrypt_by_tools)(const Utpm2DecryptParamsByTools *par, char *pwd, int *len);
    utpm2_decrypt_by_tools func = (utpm2_decrypt_by_tools)tpmLib->resolve("utpm2_decrypt_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_decrypt_by_tools failed!";
        return kLibLoadFailed;
    }

    // Extract parameters from QVariantMap
    int type = params.value(PropertyKey::kEncryptType).toInt();

    Utpm2DecryptParamsByTools pa;
    if (type == kTpmAndPcr) {
        pa.type = kCTpmAndPcr;
    } else if (type == kTpmAndPin) {
        pa.type = kCTpmAndPin;
    } else if (type == kTpmAndPcrAndPin) {
        pa.type = kCTpmAndPcrAndPin;
    } else {
        fmWarning() << "Invalid decryption type:" << type;
        return kInvalidParam;
    }

    QByteArray arrSessionHashAlgo = params.value(PropertyKey::kSessionHashAlgo).toString().toUtf8();
    pa.sessionHashAlgo = arrSessionHashAlgo.data();
    QByteArray arrSessionKeyAlgo = params.value(PropertyKey::kSessionKeyAlgo).toString().toUtf8();
    pa.sessionKeyAlgo = arrSessionKeyAlgo.data();
    QByteArray arrPriHashAlgo = params.value(PropertyKey::kPrimaryHashAlgo).toString().toUtf8();
    pa.primaryHashAlgo = arrPriHashAlgo.data();
    QByteArray arrPriKeyAlgo = params.value(PropertyKey::kPrimaryKeyAlgo).toString().toUtf8();
    pa.primaryKeyAlgo = arrPriKeyAlgo.data();
    QByteArray arrDirPath = params.value(PropertyKey::kDirPath).toString().toUtf8();
    pa.dirPath = arrDirPath.data();

    QByteArray arrPinCode = params.value(PropertyKey::kPinCode).toString().toUtf8();
    pa.pinCode = arrPinCode.data();

    QByteArray arrPcr = params.value(PropertyKey::kPcr).toString().toUtf8();
    pa.pcr = arrPcr.data();
    QByteArray arrPcrBank = params.value(PropertyKey::kPcrBank).toString().toUtf8();
    pa.pcr_bank = arrPcrBank.data();

    char password[128] = { 0 };
    int length = sizeof(password) - 1;
    int ret = func(&pa, password, &length);
    if (ret != 0) {
        fmCritical() << "utpm2_decrypt_by_tools failed with error code:" << ret;
    } else {
        *pwd = QString::fromLatin1(password);
    }

    return ret;
}

int TPMWork::ownerAuthStatus()
{
    if (!isLibraryLoaded()) {
        fmWarning() << "TPM library not loaded";
        return kLibLoadFailed;
    }

    typedef int (*utpm2_getcap_varprop_by_tools)(const char *prop_name, char *buf, size_t size);
    utpm2_getcap_varprop_by_tools func = (utpm2_getcap_varprop_by_tools)tpmLib->resolve("utpm2_getcap_varprop_by_tools");
    if (!func) {
        fmCritical() << "resolve utpm2_getcap_varprop_by_tools failed!";
        return kLibLoadFailed;
    }

    char status[100];
    int ret = func("TPM2_PT_PERMANENT.ownerAuthSet", status, sizeof(status));
    if (ret != 0) {
        fmCritical() << "cannot query ownerAuthSet, error code:" << ret;
        return -2;
    }
    return QString(status).toInt();
}
