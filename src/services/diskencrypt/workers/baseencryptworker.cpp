// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseencryptworker.h"

FILE_ENCRYPT_USE_NS

BaseEncryptWorker::BaseEncryptWorker(const QVariantMap &args, QObject *parent)
    : QThread(parent),
      m_args(args),
      m_exitCode(disk_encrypt::kSuccess)
{
}

int BaseEncryptWorker::exitCode() const
{
    return m_exitCode;
}

QVariantMap BaseEncryptWorker::args() const
{
    return m_args;
}

void BaseEncryptWorker::setExitCode(int code)
{
    QMutexLocker locker(&m_mutex);
    m_exitCode = code;
}
