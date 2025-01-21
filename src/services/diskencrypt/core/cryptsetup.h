// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRYPTSETUP_H
#define CRYPTSETUP_H

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

namespace crypt_setup {
int csInitEncrypt(const QString &dev);
int csResumeEncrypt(const QString &dev, const QString &activeName, const QString &displayName);
int csDecrypt(const QString &dev, const QString &passphrase, const QString &displayName);
int csAddPassphrase(const QString &dev, const QString &validPwd, const QString &newPwd);
int csChangePassphrase(const QString &dev, const QString &oldPwd, const QString &newPwd);
int csActivateDevice(const QString &dev, const QString &activateName);
}   // namespace crypt_setup

namespace crypt_setup_helper {
int initiable(const QString &dev);
int initFileHeader(const QString &dev, QString *fileHeader = nullptr);
int initDeviceHeader(const QString &dev, const QString &fileHeader);
int backupHeaderFile(const QString &dev, QString *fileHeader = nullptr);
int headerStatus(const QString &fileHeader);
int setToken(const QString &dev, const QString &token);
int getToken(const QString &dev, QString *token);
int onEncrypting(uint64_t size, uint64_t offset, void *usrptr);
int onDecrypting(uint64_t size, uint64_t offset, void *usrptr);
int createHeaderFile(const QString &dev, QString *headerPath);
int encryptStatus(const QString &dev);

enum HeaderStatus {
    kInvalidHeader = -1,
    kEncryptInit,
    kEncryptInProgress,
    kEncryptFully,
    kDecryptInit,
    kDecryptInProgress,
    kDecryptFully,
};
}   // namespace crypt_setup_helper

FILE_ENCRYPT_END_NS

#endif   // CRYPTSETUP_H
