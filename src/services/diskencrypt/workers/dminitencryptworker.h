// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMINITENCRYPTWORKER_H
#define DMINITENCRYPTWORKER_H

#include "baseencryptworker.h"
#include "helpers/jobfilehelper.h"

FILE_ENCRYPT_BEGIN_NS
class DMInitEncryptWorker : public BaseEncryptWorker
{
    Q_OBJECT
public:
    explicit DMInitEncryptWorker(const QVariantMap &args,
                                 QObject *parent = nullptr);

protected:
    void run() override;

    job_file_helper::JobDescArgs initJobArgs(const QString &phyDev);
};
FILE_ENCRYPT_END_NS

#endif   // DMINITENCRYPTWORKER_H
