// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTUTILS_H
#define ENCRYPTUTILS_H

#include <QString>
#include <QVariantMap>
#include <QDir>

namespace dfmmount {
class DBlockDevice;
}

typedef QSharedPointer<dfmmount::DBlockDevice> BlockDev;

namespace dfmplugin_diskenc {

namespace tpm_utils {
int checkTPM();
int checkTPMLockoutStatus();
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

// 使用 RAII 机制封装临时目录的生命周期，确保程序退出时自动清理
class TempDirHolder {
public:
    TempDirHolder();
    ~TempDirHolder();
    QString path() const;

private:
    QString m_path;
};
// 延迟初始化的单例，避免头文件中的全局构造副作用
QString getGlobalTPMConfigPath();

bool tpmSupportInterAlgo();
bool tpmSupportSMAlgo();
bool getAlgorithm(QString *sessionHashAlgo, QString *sessionKeyAlgo,
                  QString *primaryHashAlgo, QString *primaryKeyAlgo,
                  QString *minorHashAlgo, QString *minorKeyAlgo, QString *pcr, QString *pcrbank);
int genPassphraseFromTPM(const QString &dev, const QString &pin, QString *passphrase);
QString getPassphraseFromTPM(const QString &dev, const QString &pin);

int genPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin, QString *passphrase);
QString getPassphraseFromTPM_NonBlock(const QString &dev, const QString &pin);
}

namespace config_utils {
bool exportKeyEnabled();
QString cipherType();
bool enableEncrypt();
bool enableAlgoFromDConfig();
bool tpmAlgoFromDConfig(QString *sessionHash, QString *sessionKey,
                        QString *primaryHash, QString *primaryKey,
                        QString *minorHash, QString *minorKey,
                        QString *pcr, QString *pcrBank);
bool useOverlayDMMode();
}   // namespace config_utils

namespace recovery_key_utils {
QString formatRecoveryKey(const QString &raw);
}

namespace device_utils {
int encKeyType(const QString &dev);
void cacheToken(const QString &device, const QVariantMap &token);
BlockDev createBlockDevice(const QString &devObjPath);
}   // namespace device_utils

namespace dialog_utils {
int showDialog(const QString &title, const QString &msg);
int showConfirmEncryptionDialog(const QString &device, bool needReboot);
int showConfirmDecryptionDialog(const QString &device, bool needReboot);
void showTPMError(const QString &title, tpm_passphrase_utils::TPMError err);
bool isWayland();
}

}   // namespace dfmplugin_diskenc

#endif   // ENCRYPTUTILS_H
