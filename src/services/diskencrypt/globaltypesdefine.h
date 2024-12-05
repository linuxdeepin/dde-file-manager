// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALTYPESDEFINE_H
#define GLOBALTYPESDEFINE_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace disk_encrypt {
Q_NAMESPACE

inline constexpr char kEncConfigPath[] { "/boot/usec-crypt/encrypt.json" };
inline const QString kEncConfigDevicePath { "/boot/usec-crypt/encrypt_%1.json" };
inline constexpr char kReencryptDesktopFile[] { "/usr/share/applications/dfm-reencrypt.desktop" };
inline constexpr char kRebootFlagFilePrefix[] { "/tmp/dfm_encrypt_reboot_flag_" };

namespace encrypt_param_keys {
inline constexpr char kKeyDevice[] { "device" };
inline constexpr char kKeyUUID[] { "uuid" };
inline constexpr char kKeyEncMode[] { "mode" };
inline constexpr char kKeyPassphrase[] { "passphrase" };
inline constexpr char kKeyOldPassphrase[] { "oldPassphrase" };
inline constexpr char kKeyCipher[] { "cipher" };
inline constexpr char kKeyRecoveryExportPath[] { "exportRecKeyTo" };
inline constexpr char kKeyInitParamsOnly[] { "initParamsOnly" };
inline constexpr char kKeyMountPoint[] { "mountpoint" };
inline constexpr char kKeyTPMConfig[] { "tpmConfig" };
inline constexpr char kKeyTPMToken[] { "tpmToken" };
inline constexpr char kKeyValidateWithRecKey[] { "usingRecKey" };
inline constexpr char kKeyDeviceName[] { "deviceName" };
inline constexpr char kKeyBackingDevUUID[] { "backingDevUUID" };
inline constexpr char kKeyClearDevUUID[] { "clearDevUUID" };
inline constexpr char kKeyIsDetachedHeader[] { "isDetachedHeader" };
inline constexpr char kKeyPrefferDevice[] { "prefferDevice" };
inline constexpr char kKeyPartUUID[] { "partUuid" };
}   // namespace encrypt_param_keys

inline const QStringList kDisabledEncryptPath {
    "/",
    "/boot",
    "/boot/efi",
    "/recovery"
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
    kPasswordOnly,
    kTPMAndPIN,
    kTPMOnly,
};

struct DeviceEncryptParam
{
    QString devID;
    QString devDesc;
    QString uuid;
    SecKeyType type;
    QString key;
    QString newKey;
    QString exportPath;
    QString deviceDisplayName;
    QString mountPoint;
    bool initOnly;
    bool validateByRecKey;
    QString backingDevUUID;
    QString clearDevUUID;
    bool isDetachedHeader;
    QString prefferDevName;
};

struct EncryptConfig
{
    QString cipher;
    QString device;
    QString mountPoint;
    QString deviceName;
    QString devicePath;
    QString keySize;
    QString mode;
    QString recoveryPath;
    QVariantMap tpmConfig;
    QString clearDev;
    QString configPath;
    QString clearDevUUID;
    bool isDetachedHeader;
    QString fsUuid;

    QVariantMap keyConfig()
    {
        return QVariantMap {
            { "device", device },
            { "device-path", devicePath },
            { "device-name", deviceName },
            { "volume", clearDev },
            { "file-system-uuid", fsUuid }
        };
    }
};

enum EncryptState {
    kStatusNotEncrypted = 0,
    kStatusFinished = 1,
    kStatusOffline = kStatusFinished << 1,
    kStatusOnline = kStatusFinished << 2,
    kStatusEncrypt = kStatusFinished << 3,
    kStatusDecrypt = kStatusFinished << 4,
    kStatusNoEncryptConfig = kStatusFinished << 5,

    kStatusUnknown = kStatusFinished << 7,
};
Q_ENUMS(EncryptState)
Q_DECLARE_FLAGS(EncryptStates, EncryptState)
Q_DECLARE_OPERATORS_FOR_FLAGS(EncryptStates)

}

#endif   // GLOBALTYPESDEFINE_H
