// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTUTILS_H
#define ENCRYPTUTILS_H

#include <QString>
#include <QVariantMap>

namespace dfmmount {
class DBlockDevice;
}

typedef QSharedPointer<dfmmount::DBlockDevice> BlockDev;

namespace dfmplugin_diskenc {

namespace tpm_utils {
int checkTPM();
int getRandomByTPM(int size, QString *output);
int isSupportAlgoByTPM(const QString &algoName, bool *support);
int encryptByTPM(const QVariantMap &map);
int decryptByTPM(const QVariantMap &map, QString *psw);
int ownerAuthStatus();
}   // namespace tpm_utils

namespace tpm_passphrase_utils {

enum TPMError {
    kTPMNoError,
    kTPMEncryptFailed,
    kTPMLocked,
    kTPMNoRandomNumber,
    kTPMMissingAlog,
};

bool getAlgorithm(QString *sessionHashAlgo, QString *sessionKeyAlgo,
                  QString *primaryHashAlgo, QString *primaryKeyAlgo,
                  QString *minorHashAlgo, QString *minorKeyAlgo);
int genPassphraseFromTPM(const QString &dev, const QString &pin, QString *passphrase);
QString getPassphraseFromTPM(const QString &dev, const QString &pin);

int genPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin, QString *passphrase);
QString getPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin);
}

namespace config_utils {
bool exportKeyEnabled();
QString cipherType();
bool enableEncrypt();
}   // namespace config_utils

namespace recovery_key_utils {
QString formatRecoveryKey(const QString &raw);
}

namespace fstab_utils {
bool isFstabItem(const QString &mpt);
}   // namespace fstab_utils

namespace device_utils {
int encKeyType(const QString &dev);
void cacheToken(const QString &device, const QVariantMap &token);
BlockDev createBlockDevice(const QString &devObjPath);
}   // namespace device_utils

namespace dialog_utils {
enum DialogType {
    kInfo,
    kWarning,
    kError,
};
int showDialog(const QString &title, const QString &msg, DialogType type);
int showConfirmEncryptionDialog(const QString &device, bool needReboot);
int showConfirmDecryptionDialog(const QString &device, bool needReboot);
void showTPMError(const QString &title, tpm_passphrase_utils::TPMError err);
bool isWayland();
}

}   // namespace dfmplugin_diskenc

#endif   // ENCRYPTUTILS_H
