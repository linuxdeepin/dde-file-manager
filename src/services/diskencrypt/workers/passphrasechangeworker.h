// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PASSPHRASECHANGEWORKER_H
#define PASSPHRASECHANGEWORKER_H

#include "diskencrypt_global.h"
#include "baseencryptworker.h"

FILE_ENCRYPT_BEGIN_NS

class PassphraseChangeWorker : public BaseEncryptWorker
{
public:
    explicit PassphraseChangeWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;
};

FILE_ENCRYPT_END_NS

#endif   // PASSPHRASECHANGEWORKER_H
