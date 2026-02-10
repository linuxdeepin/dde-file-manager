// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEENCRYPTWORKER_H
#define BASEENCRYPTWORKER_H

#include "diskencrypt_global.h"

#include <QThread>
#include <QMutex>

FILE_ENCRYPT_BEGIN_NS

class BaseEncryptWorker : public QThread
{
    Q_OBJECT
public:
    explicit BaseEncryptWorker(const QVariantMap &args, QObject *parent = nullptr);
    int exitCode() const;
    QVariantMap args() const;

protected:
    void setExitCode(int);

protected:
    QVariantMap m_args;

private:
    int m_exitCode;
    QMutex m_mutex;
};

FILE_ENCRYPT_END_NS

#endif   // BASEENCRYPTWORKER_H
