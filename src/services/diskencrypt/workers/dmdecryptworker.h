// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMDECRYPTWORKER_H
#define DMDECRYPTWORKER_H
#include "diskencrypt_global.h"
#include "baseencryptworker.h"

FILE_ENCRYPT_BEGIN_NS

class DMDecryptWorker : public BaseEncryptWorker
{
public:
    explicit DMDecryptWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;

    void getDevPath(QString *phyDev, QString *clearDev);
};

FILE_ENCRYPT_END_NS

#endif   // DMDECRYPTWORKER_H
