// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRYPTSETUPCOMPABILITYHELPER_H
#define CRYPTSETUPCOMPABILITYHELPER_H

#include "diskencrypt_global.h"

#include <QObject>
#include <QLibrary>

#include <libcryptsetup.h>

typedef int (*InitWithPreProcess)(struct crypt_device *cd,
                                  const char *name,
                                  const char *passphrase,
                                  size_t passphrase_size,
                                  int keyslot_old,
                                  int keyslot_new,
                                  const char *cipher,
                                  const char *cipher_mode,
                                  const struct crypt_params_reencrypt *params,
                                  int (*data_device_preprocess)(int, const char *[]),
                                  int process_argc,
                                  const char *process_argv[]);

FILE_ENCRYPT_BEGIN_NS

class CryptSetupCompabilityHelper : public QObject
{
    Q_OBJECT

public:
    static CryptSetupCompabilityHelper *instance();
    InitWithPreProcess initWithPreProcess() const { return m_func; }

protected:
    explicit CryptSetupCompabilityHelper(QObject *parent = nullptr);
    ~CryptSetupCompabilityHelper();

private:
    InitWithPreProcess m_func { nullptr };
    QLibrary *m_libCryptsetup { nullptr };
};

FILE_ENCRYPT_END_NS

#endif   // CRYPTSETUPCOMPABILITYHELPER_H
