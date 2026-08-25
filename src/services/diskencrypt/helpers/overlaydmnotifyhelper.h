// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OVERLAYDMNOTIFYHELPER_H
#define OVERLAYDMNOTIFYHELPER_H

#include <QObject>

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

class OverlayDMNotifyHelper : public QObject
{
    Q_OBJECT

public:
    static OverlayDMNotifyHelper *instance();

    void ensureNotificationDelivery(bool enabled, int result);

private:
    struct ActiveUser {
        uint uid { 0 };
        QString username;
        bool valid { false };
    };

    explicit OverlayDMNotifyHelper(QObject *parent = nullptr);

    ActiveUser findActiveGraphicalUser();
    bool isFileManagerRunning(uint uid);
    void launchFileManager(uint uid, const QString &username);
    void writePendingFile(bool enabled, int result, const ActiveUser &user);
};

FILE_ENCRYPT_END_NS

#endif   // OVERLAYDMNOTIFYHELPER_H
