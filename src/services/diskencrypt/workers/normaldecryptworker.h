// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALDECRYPTWORKER_H
#define NORMALDECRYPTWORKER_H

#include "diskencrypt_global.h"
#include "baseencryptworker.h"

FILE_ENCRYPT_BEGIN_NS

class NormalDecryptWorker : public BaseEncryptWorker
{
public:
    explicit NormalDecryptWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;
};

FILE_ENCRYPT_END_NS

#endif   // NORMALDECRYPTWORKER_H
