// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmdecryptworker.h"

FILE_ENCRYPT_USE_NS

DMDecryptWorker::DMDecryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMDecryptWorker::run()
{
    qWarning() << "DM DECRYPT NOT SUPPORTED YET";
}
