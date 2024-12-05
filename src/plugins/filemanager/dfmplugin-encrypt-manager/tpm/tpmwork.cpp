// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tpmwork.h"

#include <QLibrary>
#include <QDebug>
#include <QFile>
#include <QDir>

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

inline constexpr int kTpmOutTextMaxSize { 3000 };
inline constexpr char kTpmLibName[] { "libutpm2.so" };
inline constexpr char kTpmEncryptFileName[] { "tpm_encrypt.txt" };

DPENCRYPTMANAGER_USE_NAMESPACE

TPMWork::TPMWork(QObject *parent)
    : QObject(parent), tpmLib(new QLibrary(kTpmLibName))
{
    if (!tpmLib->load())
        qWarning() << "Vault: load utpm2 failed, the error is " << tpmLib->errorString();
}

TPMWork::~TPMWork()
{
    if (tpmLib) {
        tpmLib->unload();
        delete tpmLib;
        tpmLib = nullptr;
    }
}

bool TPMWork::checkTPMAvailable()
{
    if (!tpmLib->isLoaded())
        return false;

    QString output;
    return getRandom(2, &output);
}

bool TPMWork::getRandom(int size, QString *output)
{
    if (!tpmLib->isLoaded())
        return false;

    if (size % 2 != 0 || size < 2 || size > 64) {
        qCritical() << "Vault: random size must be even and greater than or equal to 2 and less than or equal to 64!";
        return false;
    }

    typedef bool (*p_get_random)(uint16_t * size, uint8_t ranbytes[]);
    p_get_random utpm2_get_random = (p_get_random)tpmLib->resolve("utpm2_get_random");
    if (utpm2_get_random) {
        uint16_t len = size / 2;
        uint8_t *random = (uint8_t *)malloc(sizeof(uint8_t) * len);
        memset(random, 0, sizeof(uint8_t) * len);
        if (utpm2_get_random(&len, random)) {
            char *out = (char *)malloc(sizeof(uint8_t) * size + 1);
            memset(out, 0, sizeof(uint8_t) * size + 1);
            for (size_t i = 0; i < len; ++i) {
                sprintf(out + (i * 2), "%02x", random[i]);
            }
            *output = QString(out);
            free(random);
            free(out);
            return true;
        }
    }
    return false;
}

bool TPMWork::isSupportAlgo(const QString &algoName, bool *support)
{
    if (!tpmLib->isLoaded())
        return false;

    typedef bool (*p_check_algo)(const char *alg);
    p_check_algo utpm2_check_algo = (p_check_algo)tpmLib->resolve("utpm2_check_alg");
    if (utpm2_check_algo) {
        QByteArray arAlgoName = algoName.toUtf8();
        if (utpm2_check_algo(arAlgoName.data())) {
            *support = true;
        } else {
            *support = false;
        }
        return true;
    }

    return false;
}

bool TPMWork::initTpm2(const QString &hashAlgo, const QString &keyAlgo, const QString &keyPin, const QString &dirPath)
{
    if (!tpmLib->isLoaded())
        return false;

    typedef bool (*p_init)(char *algdetail, char *galg, const char *auth, const char *dir);
    p_init utpm2_init = (p_init)tpmLib->resolve("utpm2_init");
    if (utpm2_init) {
        QByteArray arKeyAlgo = keyAlgo.toUtf8();
        QByteArray arHashAlgo = hashAlgo.toUtf8();
        QByteArray arKeyPin = keyPin.toUtf8();
        QByteArray arDir = dirPath.toUtf8();
        if (utpm2_init(arKeyAlgo.data(), arHashAlgo.data(), arKeyPin.data(), arDir.data())) {
            return true;
        } else {
            qCritical() << "Vault: utpm2_init return false!";
        }
    } else {
        qCritical() << "Vault: resolve utpm2_init failed!";
    }
    return false;
}

bool TPMWork::encrypt(const QString &hashAlgo, const QString &keyAlgo, const QString &keyPin, const QString &password, const QString &dirPath)
{
    if (!initTpm2(hashAlgo, keyAlgo, keyPin, dirPath)) {
        return false;
    }

    typedef int (*p_encrypt_decrypt)(const char *dir, bool isdecrypt, const char *auth, uint8_t inbytes[], uint8_t outbytes[], uint16_t *size);
    p_encrypt_decrypt utpm2_encrypt_decrypt = (p_encrypt_decrypt)tpmLib->resolve("utpm2_encrypt_decrypt");
    if (utpm2_encrypt_decrypt) {
        QByteArray arDir = dirPath.toUtf8();
        QByteArray arKeyPin = keyPin.toUtf8();
        QByteArray arrPassword = password.toUtf8();
        uint16_t len = static_cast<uint16_t>(arrPassword.size());
        uint8_t *inbuffer = reinterpret_cast<uint8_t *>(arrPassword.data());
        uint8_t out_text[kTpmOutTextMaxSize] = { 0 };
        if (utpm2_encrypt_decrypt(arDir.data(), false, arKeyPin.data(), inbuffer, out_text, &len)) {
            QFile file(dirPath + QDir::separator() + kTpmEncryptFileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(reinterpret_cast<const char *>(out_text), len);
                file.close();
                return true;
            } else {
                qCritical() << "Vault: open encrypt file failed!";
            }
        } else {
            qCritical() << "Vault: utpm2_encry_decrypt return false!";
        }
    } else {
        qCritical() << "Vault: resolve utpm2_encry_decrypt failed!";
    }
    return false;
}

bool TPMWork::decrypt(const QString &keyPin, const QString &dirPath, QString *psw)
{
    if (!tpmLib->isLoaded())
        return false;

    typedef int (*p_encrypt_decrypt)(const char *dir, bool isdecrypt, const char *auth, uint8_t inbytes[], uint8_t outbytes[], uint16_t *size);
    p_encrypt_decrypt utpm2_encrypt_decrypt = (p_encrypt_decrypt)tpmLib->resolve("utpm2_encrypt_decrypt");
    if (utpm2_encrypt_decrypt) {
        QByteArray arDir = QString(dirPath).toUtf8();
        QByteArray arKeyPin = keyPin.toUtf8();
        QFile file(dirPath + QDir::separator() + kTpmEncryptFileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ciphertext = file.readAll();
            file.close();
            uint16_t len = static_cast<uint16_t>(ciphertext.size());
            uint8_t out_text[kTpmOutTextMaxSize] = { 0 };
            uint8_t *inbuffer = reinterpret_cast<uint8_t *>(ciphertext.data());
            if (utpm2_encrypt_decrypt(arDir.data(), true, arKeyPin.data(), inbuffer, out_text, &len)) {
                *psw = QString::fromUtf8(reinterpret_cast<const char *>(out_text), len);
                return true;
            } else {
                qCritical() << "Vault: utpm2_encry_decrypt return failed!";
            }
        } else {
            qCritical() << "Vault: open encrypt file failed!";
        }
    } else {
        qCritical() << "Vault: resolve utpm2_encry_decrypt failed!";
    }
    return false;
}

int TPMWork::checkTPMAvailbableByTools()
{
    if (!tpmLib->isLoaded())
        return -1;

    typedef int (*p_utpm2_check_tpm_by_tools)(void);
    p_utpm2_check_tpm_by_tools utpm2_check_tpm_by_tools = (p_utpm2_check_tpm_by_tools)tpmLib->resolve("utpm2_check_tpm_by_tools");
    if (!utpm2_check_tpm_by_tools) {
        qCritical() << "resolve utpm2_check_tpm_by_tools failed!";
        return -1;
    }

    return utpm2_check_tpm_by_tools();
}

int TPMWork::getRandomByTools(int size, QString *output)
{
    if (!tpmLib->isLoaded())
        return -1;

    typedef int (*p_utpm2_get_random_by_tools)(int size, char *buf);
    p_utpm2_get_random_by_tools utpm2_get_random_by_tools = (p_utpm2_get_random_by_tools)tpmLib->resolve("utpm2_get_random_by_tools");
    if (!utpm2_get_random_by_tools) {
        qCritical() << "resolve utpm2_get_random_by_tools failed!";
        return -1;
    }

    char buf[129] = { 0 };
    int re = utpm2_get_random_by_tools(size, buf);
    *output = QString::fromLatin1(buf);
    return re;
}

int TPMWork::isSupportAlgoByTools(const QString &algoName, bool *support)
{
    if (!tpmLib->isLoaded())
        return -1;

    typedef int (*p_utpm2_check_alg_by_tools)(const char *algo_name, bool *support);
    p_utpm2_check_alg_by_tools utpm2_check_alg_by_tools = (p_utpm2_check_alg_by_tools)tpmLib->resolve("utpm2_check_alg_by_tools");
    if (!utpm2_check_alg_by_tools) {
        qCritical() << "resolve utpm2_check_alg_by_tools failed!";
        return -1;
    }

    QByteArray algo = algoName.toUtf8();
    int re = utpm2_check_alg_by_tools(algo.data(), support);
    return re;
}

int TPMWork::encryptByTools(const EncryptParams &params)
{
    if (!tpmLib->isLoaded())
        return -1;

    typedef int (*utpm2_encrypt_by_tools)(const Utpm2EncryptParamsByTools *par);
    utpm2_encrypt_by_tools func = (utpm2_encrypt_by_tools)tpmLib->resolve("utpm2_encrypt_by_tools");
    if (!func) {
        qCritical() << "resolve utpm2_encrypt_by_tools failed!";
        return -1;
    }

    Utpm2EncryptParamsByTools pa;
    if (params.type == kTpmAndPcr) {
        pa.type = kCTpmAndPcr;
    } else if (params.type == kTpmAndPin) {
        pa.type = kCTpmAndPin;
    } else if (params.type == kTpmAndPcrAndPin) {
        pa.type = kCTpmAndPcrAndPin;
    } else {
        return -1;
    }
    QByteArray arrSessionHashAlgo = params.sessionHashAlgo.toUtf8();
    pa.sessionHashAlgo = arrSessionHashAlgo.data();
    QByteArray arrSessionKeyAlgo = params.sessionKeyAlgo.toUtf8();
    pa.sessionKeyAlgo = arrSessionKeyAlgo.data();
    QByteArray arrPriHashAlgo = params.primaryHashAlgo.toUtf8();
    pa.primaryHashAlgo = arrPriHashAlgo.data();
    QByteArray arrPriKeyAlgo = params.primaryKeyAlgo.toUtf8();
    pa.primaryKeyAlgo = arrPriKeyAlgo.data();
    QByteArray arrMinHashAlgo = params.minorHashAlgo.toUtf8();
    pa.minorHashAlgo = arrMinHashAlgo.data();
    QByteArray arrMinKeyAlgo = params.minorKeyAlgo.toUtf8();
    pa.minorKeyAlgo = arrMinKeyAlgo.data();

    QByteArray arrDirPath = params.dirPath.toUtf8();
    pa.dirPath = arrDirPath.data();
    QByteArray arrPlain = params.plain.toUtf8();
    pa.plain = arrPlain.data();

    QByteArray arrPinCode = params.pinCode.toUtf8();
    pa.pinCode = arrPinCode.data();

    QByteArray arrPcr = params.pcr.toUtf8();
    pa.pcr = arrPcr.data();
    QByteArray arrPcrBank = params.pcr_bank.toUtf8();
    pa.pcr_bank = arrPcrBank.data();

    int re = func(&pa);
    if (re != 0) {
        qCritical() << "utpm2_encrypt_by_tools return false!";
    }

    return re;
}

int TPMWork::decryptByTools(const DecryptParams &params, QString *pwd)
{
    if (!tpmLib->isLoaded())
        return -1;

    typedef int (*utpm2_decrypt_by_tools)(const Utpm2DecryptParamsByTools *par, char *pwd, int *len);
    utpm2_decrypt_by_tools fun = (utpm2_decrypt_by_tools)tpmLib->resolve("utpm2_decrypt_by_tools");
    if (!fun) {
        qCritical() << "resolve utpm2_encry_decrypt failed!";
        return -1;
    }

    Utpm2DecryptParamsByTools pa;
    if (params.type == kTpmAndPcr) {
        pa.type = kCTpmAndPcr;
    } else if (params.type == kTpmAndPin) {
        pa.type = kCTpmAndPin;
    } else if (params.type == kTpmAndPcrAndPin) {
        pa.type = kCTpmAndPcrAndPin;
    } else {
        return -1;
    }
    QByteArray arrSessionHashAlgo = params.sessionHashAlgo.toUtf8();
    pa.sessionHashAlgo = arrSessionHashAlgo.data();
    QByteArray arrSessionKeyAlgo = params.sessionKeyAlgo.toUtf8();
    pa.sessionKeyAlgo = arrSessionKeyAlgo.data();
    QByteArray arrPriHashAlgo = params.primaryHashAlgo.toUtf8();
    pa.primaryHashAlgo = arrPriHashAlgo.data();
    QByteArray arrPriKeyAlgo = params.primaryKeyAlgo.toUtf8();
    pa.primaryKeyAlgo = arrPriKeyAlgo.data();
    QByteArray arrDirPath = params.dirPath.toUtf8();
    pa.dirPath = arrDirPath.data();

    QByteArray arrPinCode = params.pinCode.toUtf8();
    pa.pinCode = arrPinCode.data();

    QByteArray arrPcr = params.pcr.toUtf8();
    pa.pcr = arrPcr.data();
    QByteArray arrPcrBank = params.pcr_bank.toUtf8();
    pa.pcr_bank = arrPcrBank.data();

    char password[128] = { 0 };
    int length = sizeof(password) - 1;
    int re = fun(&pa, password, &length);
    if (re != 0) {
        qCritical() << "utpm2_encry_decrypt return failed!";
    }
    (*pwd) = QString::fromLatin1(password);

    return re;
}

int TPMWork::ownerAuthStatus()
{
    typedef int (*utpm2_getcap_varprop_by_tools)(const char *prop_name, char *buf, size_t size);
    utpm2_getcap_varprop_by_tools fun = (utpm2_getcap_varprop_by_tools)tpmLib->resolve("utpm2_getcap_varprop_by_tools");
    if (!fun) {
        qCritical() << "resolve utpm2_encry_decrypt failed!";
        return -1;
    }
    char status[100];
    int ret = fun("TPM2_PT_PERMANENT.ownerAuthSet", status, sizeof(status));
    if (ret != 0) {
        qCritical() << "cannot query ownerAuthSet";
        return -2;
    }
    return QString(status).toInt();
}
