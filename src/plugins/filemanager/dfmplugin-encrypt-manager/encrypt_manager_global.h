// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPT_MANAGER_GLOBAL_H
#define ENCRYPT_MANAGER_GLOBAL_H

#define DPENCRYPTMANAGER_NAMESPACE dfmplugin_encrypt_manager
#define DPENCRYPTMANAGER_BEGIN_NAMESPACE namespace dfmplugin_encrypt_manager {
#define DPENCRYPTMANAGER_END_NAME }
#define DPENCRYPTMANAGER_USE_NAMESPACE using namespace dfmplugin_encrypt_manager;

#include <QString>

namespace dfmplugin_encrypt_manager {

enum TPMType {
    kUnknow = 0,
    kTpmAndPcr,
    kTpmAndPin,
    kTpmAndPcrAndPin
};

struct EncryptParams
{
    TPMType type;
    QString sessionHashAlgo;
    QString sessionKeyAlgo;
    QString primaryHashAlgo;
    QString primaryKeyAlgo;
    QString minorHashAlgo;
    QString minorKeyAlgo;
    QString dirPath;
    QString plain;

    QString pinCode;

    QString pcr;
    QString pcr_bank;
};

struct DecryptParams
{
    TPMType type;
    QString sessionHashAlgo;
    QString sessionKeyAlgo;
    QString primaryHashAlgo;
    QString primaryKeyAlgo;
    QString dirPath;

    QString pinCode;

    QString pcr;
    QString pcr_bank;
};

namespace PropertyKey {
inline constexpr char kEncryptType[] { "PropertyKey_EncryptType" };
inline constexpr char kSessionHashAlgo[] { "PropertyKey_SessionHashAlgo" };
inline constexpr char kSessionKeyAlgo[] { "PropertyKey_SessionKeyAlgo" };
inline constexpr char kPrimaryHashAlgo[] { "PropertyKey_PrimaryHashAlgo" };
inline constexpr char kPrimaryKeyAlgo[] { "PropertyKey_PrimaryKeyAlgo" };
inline constexpr char kMinorHashAlgo[] { "PropertyKey_MinorHashAlgo" };
inline constexpr char kMinorKeyAlgo[] { "PropertyKey_MinorKeyAlgo" };
inline constexpr char kDirPath[] { "PropertyKey_DirPath" };
inline constexpr char kPlain[] { "PropertyKey_Plain" };
inline constexpr char kPinCode[] { "PropertyKey_PinCode" };
inline constexpr char kPcr[] { "PropertyKey_Pcr" };
inline constexpr char kPcrBank[] { "PropertyKey_PcrBank" };
}

}

#endif // ENCRYPT_MANAGER_GLOBAL_H
