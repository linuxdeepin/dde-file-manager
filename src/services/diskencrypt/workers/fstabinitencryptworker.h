// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSTABINITENCRYPTWORKER_H
#define FSTABINITENCRYPTWORKER_H

#include "baseencryptworker.h"
#include "helpers/jobfilehelper.h"

FILE_ENCRYPT_BEGIN_NS

class FstabInitEncryptWorker : public BaseEncryptWorker
{
    Q_OBJECT
public:
    explicit FstabInitEncryptWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;

    job_file_helper::JobDescArgs initJobArgs(const QString &dev);
};

FILE_ENCRYPT_END_NS
#endif   // FSTABINITENCRYPTWORKER_H
