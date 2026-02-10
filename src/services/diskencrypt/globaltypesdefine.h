// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALTYPESDEFINE_H
#define GLOBALTYPESDEFINE_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVariantMap>

namespace disk_encrypt {
Q_NAMESPACE

static QString fromBase64(const QString b64str)
{
    return QByteArray::fromBase64(b64str.toLocal8Bit());
}

static QString toBase64(const QString rawstr)
{
    return rawstr.toLocal8Bit().toBase64();
}

inline constexpr char kUSecConfigDir[] { "/etc/usec-crypt" };
inline constexpr char kReencryptDesktopFile[] { "/usr/local/share/applications/dfm-reencrypt.desktop" };
inline constexpr char kRebootFlagFilePrefix[] { "/tmp/dfm_encrypt_reboot_flag_" };

namespace job_type {
static const char *TypeFstab { "fstab" };
static const char *TypeOverlay { "usec-overlay" };
static const char *TypeNormal { "normal" };
}   // namespace job_type

namespace encrypt_param_keys {
inline constexpr char kKeyDevice[] { "device-path" };
inline constexpr char kKeyPhyDevice[] { "physical-device-path" };
inline constexpr char kKeyUUID[] { "uuid" };
inline constexpr char kKeyPassphrase[] { "passphrase" };
inline constexpr char kKeyOldPassphrase[] { "old-passphrase" };
inline constexpr char kKeyExportToPath[] { "export-target-path" };
inline constexpr char kKeyMountPoint[] { "mountpoint" };
inline constexpr char kKeyTPMToken[] { "tpm-token" };
inline constexpr char kKeyDeviceName[] { "device-name" };
inline constexpr char kKeyOperationResult[] { "operation-result" };
inline constexpr char kKeyRecoveryKey[] { "recovery-key" };
inline constexpr char kKeyJobType[] { "job-type" };
inline constexpr char kKeyValidateWithRecKey[] { "validate-with-reckey" };
}   // namespace encrypt_param_keys

inline const QStringList kDisabledEncryptPath {
    // "/",
    "/boot",
    "/boot/efi",
    "/recovery",
    // "/sysroot"
};

enum EncryptOperationStatus {
    kSuccess = 0,
    kUserCancelled,
    kRebootRequired,
    kIgnoreRequest,

    kErrorParamsInvalid,
    kErrorDeviceMounted,
    kErrorDeviceEncrypted,
    kErrorWrongPassphrase,
    kErrorEncryptBusy,
    kErrorCannotStartEncryptJob,
    kErrorCreateHeader,
    kErrorHeaderNotExist,
    kErrorBackupHeader,
    kErrorApplyHeader,
    kErrorInitCrypt,
    kErrorInitReencrypt,
    kErrorRestoreFromFile,
    kErrorActive,
    kErrorDeactive,
    kErrorLoadCrypt,
    kErrorGetReencryptFlag,
    kErrorWrongFlags,
    kErrorSetOffset,
    kErrorFormatLuks,
    kErrorAddKeyslot,
    kErrorReencryptFailed,
    kErrorDecryptFailed,
    kErrorOpenFstabFailed,
    kErrorChangePassphraseFailed,
    kErrorOpenFileFailed,
    kErrorSetTokenFailed,
    kErrorResizeFs,
    kErrorDisabledMountPoint,
    kErrorSetLabel,
    kErrorNotFullyEncrypted,
    KErrorRequestExportRecKey,
    kErrorSetFsPassno,
    kErrorCheckReencryptStatus,

    kErrorUnknown,
};

enum SecKeyType {
    kPwd,
    kPin,
    kTpm,
};

enum EncryptState {
    kStatusNotEncrypted = 0,
    kStatusFinished = 1,
    kStatusOffline = kStatusFinished << 1,
    kStatusOnline = kStatusFinished << 2,
    kStatusEncrypt = kStatusFinished << 3,
    kStatusDecrypt = kStatusFinished << 4,
    kStatusUnknown = kStatusFinished << 7,
};
Q_ENUMS(EncryptState)
Q_DECLARE_FLAGS(EncryptStates, EncryptState)
Q_DECLARE_OPERATORS_FOR_FLAGS(EncryptStates)

struct DeviceEncryptParam
{
    QString devID;
    QString devDesc;
    QString devPhy;
    QString jobType;
    QString devPreferPath;
    QString key;
    QString newKey;
    QString exportPath;
    EncryptStates states;
    SecKeyType secType;
    QString deviceDisplayName;
    QString mountPoint;
    bool validateByRecKey;
};

}

#endif   // GLOBALTYPESDEFINE_H
