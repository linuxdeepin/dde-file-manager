// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSTABDECRYPTWORKER_H
#define FSTABDECRYPTWORKER_H

#include "diskencrypt_global.h"
#include "baseencryptworker.h"

FILE_ENCRYPT_BEGIN_NS

class FstabDecryptWorker : public BaseEncryptWorker
{
public:
    explicit FstabDecryptWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;

    QString clearDeviceUUID(const QString &dev);
    QString partitionUUID(const QString &dev);
};

FILE_ENCRYPT_END_NS
#endif   // FSTABDECRYPTWORKER_H
