// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include "diskencrypt_global.h"

#include <QObject>

FILE_ENCRYPT_BEGIN_NS

class SignalEmitter : public QObject
{
    Q_OBJECT

public:
    static SignalEmitter *instance();

Q_SIGNALS:
    void updateEncryptProgress(const QString &dev, double progress);
    void updateDecryptProgress(const QString &dev, double progress);
};

FILE_ENCRYPT_END_NS

#endif   // NOTIFICATIONS_H
