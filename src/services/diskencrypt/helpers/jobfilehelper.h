// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOBFILEHELPER_H
#define JOBFILEHELPER_H

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS
namespace job_file_helper {

namespace key_name {
static const char *KeyDevice { "device" };
static const char *KeyVolume { "volume" };
static const char *KeyCipher { "cipher" };
static const char *KeyKeySze { "key-size" };
static const char *KeyDevPath { "device-path" };
static const char *KeyPrefferPath { "preffer-path" };
static const char *KeyDevType { "device-type" };
static const char *KeyDevName { "device-name" };
static const char *KeyClearDev { "clear-uuid" };
}   // namespace key_name

struct JobDescArgs
{
    QString device;   // "device": "PARTUUID=XXXXXX"
    QString volume;   // "volume": "usec-overlay-unlocked-", the name that presents the unlocked device
    QString cipher;   // "cipher": "sm4-xts-plain64", or aes...
    QString keySze;   // "key-size": "256", the encrypt key size
    // above fields are usec needed.

    // below are for dde-file-manager only.
    QString devPath;   // "device-path": "/dev/nvme0n1p1"
    QString prefferPath;   // "preffer-path": "/dev/nvme0n1p1", might be different if mapper device
    QString devType;   // "device-type": "fstab", one of job_type
    QString devName;   // "device-name": "System Disk", device display name.
    QString clearDev;   // "clear-uuid": "UUID=XXXXXX", used to identify the unlocked device, will be used when decrypt
    QString jobFile;   // the absolute path of job file.
};

int createUSecRoot();
int createEncryptJobFile(JobDescArgs &args);
int createDecryptJobFile(JobDescArgs &args);
int loadEncryptJobFile(JobDescArgs *args, const QString &dev = QString());
// int loadDecryptJobFile(JobDescArgs *args = nullptr);
int removeJobFile(const QString &jobFile);
void checkJobs();

bool hasJobFile();
QStringList validJobTypes();
}   // namespace job_file_helper
FILE_ENCRYPT_END_NS

#endif   // JOBFILEHELPER_H
