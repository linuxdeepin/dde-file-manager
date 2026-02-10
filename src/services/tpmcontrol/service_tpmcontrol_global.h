// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_TPMCONTROL_GLOBAL_H
#define SERVICE_TPMCONTROL_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define SERVICETPMCONTROL_NAMESPACE service_tpmcontrol
#define SERVICETPMCONTROL_BEGIN_NAMESPACE namespace SERVICETPMCONTROL_NAMESPACE {
#define SERVICETPMCONTROL_END_NAMESPACE }
#define SERVICETPMCONTROL_USE_NAMESPACE using namespace SERVICETPMCONTROL_NAMESPACE;

SERVICETPMCONTROL_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(SERVICETPMCONTROL_NAMESPACE)

// TPM encryption types
enum TPMType {
    kUnknow = 0,
    kTpmAndPcr = 1,        // TPM + PCR only
    kTpmAndPin = 2,        // TPM + PIN
    kTpmAndPcrAndPin = 3   // TPM + PCR + PIN
};

// Error codes (use negative values to avoid confusion with errno)
enum TPMError {
    kNoError = 0,
    kAuthFailed = -1,           // PolicyKit authentication failed
    kTPMNotAvailable = -2,      // TPM device not available
    kTPMLocked = -3,            // TPM is in lockout state
    kInvalidParam = -4,         // Invalid parameter
    kAlgoNotSupport = -5,       // Algorithm not supported
    kEncryptFailed = -6,        // Encryption failed
    kDecryptFailed = -7,        // Decryption failed
    kLibLoadFailed = -8,        // Failed to load libutpm2.so
    kFdCreateFailed = -9,       // Failed to create file descriptor
    kFdWriteFailed = -10,       // Failed to write to file descriptor
    kFdReadFailed = -11,        // Failed to read from file descriptor
};

// Property keys for QVariantMap parameters
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

// PolicyKit action IDs
namespace PolicyKitActionId {
inline constexpr char kQuery[] { "org.deepin.Filemanager.TPMControl.Query" };
inline constexpr char kEncrypt[] { "org.deepin.Filemanager.TPMControl.Encrypt" };
inline constexpr char kDecrypt[] { "org.deepin.Filemanager.TPMControl.Decrypt" };
}

SERVICETPMCONTROL_END_NAMESPACE

#endif   // SERVICE_TPMCONTROL_GLOBAL_H
