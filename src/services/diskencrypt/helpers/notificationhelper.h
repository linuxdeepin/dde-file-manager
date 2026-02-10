// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

#include <QObject>

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

class NotificationHelper : public QObject
{
    Q_OBJECT

public:
    static NotificationHelper *instance();

Q_SIGNALS:
    void notifyEncryptProgress(const QString &dev, const QString &name, double progress);
    void notifyDecryptProgress(const QString &dev, const QString &name, double progress);
    void replyAuthArgs(const QVariantMap &args);
    void ignoreAuthSetup();
};

FILE_ENCRYPT_END_NS

#endif   // NOTIFICATIONHELPER_H
