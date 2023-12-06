// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TPMWORK_H
#define TPMWORK_H

#include "serverplugin_vaultdaemon_global.h"

#include <QThread>

namespace serverplugin_vaultdaemon {
class TpmWork : public QThread
{
    Q_OBJECT
public:
    explicit TpmWork(QObject *parent = nullptr);
    ~TpmWork() override;
    void run() override;

Q_SIGNALS:
    void workFinished(TpmDecryptState state, const QString &passwd);
};
}

#endif // TPMWORK_H
