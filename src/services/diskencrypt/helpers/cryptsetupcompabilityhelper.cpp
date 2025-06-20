// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cryptsetupcompabilityhelper.h"

#include <QLibrary>

FILE_ENCRYPT_BEGIN_NS
class InstanceHelper : public CryptSetupCompabilityHelper
{
};
FILE_ENCRYPT_END_NS

FILE_ENCRYPT_USE_NS

Q_GLOBAL_STATIC(InstanceHelper, helper);

CryptSetupCompabilityHelper *CryptSetupCompabilityHelper::instance()
{
    return helper();
}

CryptSetupCompabilityHelper::CryptSetupCompabilityHelper(QObject *parent)
    : QObject { parent }
{
    qInfo() << "[CryptSetupCompabilityHelper] Initializing cryptsetup compatibility helper";
    
    m_libCryptsetup = new QLibrary("libcryptsetup.so.12", this);
    if (!m_libCryptsetup->load()) {
        qCritical() << "[CryptSetupCompabilityHelper] Failed to load cryptsetup library";
        return;
    }
    
    qInfo() << "[CryptSetupCompabilityHelper] Cryptsetup library loaded successfully";
    
    auto name = "crypt_reencrypt_init_by_passphrase_with_data_device_preprocess";
    m_func = (InitWithPreProcess)m_libCryptsetup->resolve(name);
    if (!m_func) {
        qWarning() << "[CryptSetupCompabilityHelper] Current cryptsetup version does not provide expected function:" << name;
    } else {
        qInfo() << "[CryptSetupCompabilityHelper] Cryptsetup new API function found and resolved successfully";
    }
}

CryptSetupCompabilityHelper::~CryptSetupCompabilityHelper()
{
    qInfo() << "[CryptSetupCompabilityHelper] Destroying cryptsetup compatibility helper";
    
    if (m_libCryptsetup && m_libCryptsetup->isLoaded()) {
        m_libCryptsetup->unload();
        qInfo() << "[CryptSetupCompabilityHelper] Cryptsetup library unloaded successfully";
    }
}
