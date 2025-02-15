// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALINITENCRYPTWORKER_H
#define NORMALINITENCRYPTWORKER_H

#include "baseencryptworker.h"

#include "helpers/jobfilehelper.h"
#include "helpers/blockdevhelper.h"

FILE_ENCRYPT_BEGIN_NS

class NormalInitEncryptWorker : public BaseEncryptWorker
{
    Q_OBJECT
public:
    explicit NormalInitEncryptWorker(const QVariantMap &args, QObject *parent = nullptr);

protected:
    void run() override;
    job_file_helper::JobDescArgs initJobArgs(DevPtr ptr);
};

FILE_ENCRYPT_END_NS
#endif   // NORMALINITENCRYPTWORKER_H
