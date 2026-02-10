// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibithelper.h"

#include <QDBusInterface>
#include <QDebug>

FILE_ENCRYPT_USE_NS

QDBusReply<QDBusUnixFileDescriptor> inhibit_helper::inhibit(const QString &message)
{
    qInfo() << "[inhibit_helper::inhibit] Creating system inhibit lock with message:" << message;
    
    QDBusInterface iface("org.freedesktop.login1",
                         "/org/freedesktop/login1",
                         "org.freedesktop.login1.Manager",
                         QDBusConnection::systemBus());
    QVariantList args;
    args << QString("shutdown:sleep")
         << QString("file-manager-daemon")
         << QString(message)
         << QString("block");

    QDBusReply<QDBusUnixFileDescriptor> ret = iface.callWithArgumentList(QDBus::Block, "Inhibit", args);
    
    if (ret.isValid()) {
        qInfo() << "[inhibit_helper::inhibit] System inhibit lock created successfully - message:" << message << "fd:" << ret.value().fileDescriptor();
    } else {
        qWarning() << "[inhibit_helper::inhibit] Failed to create system inhibit lock - message:" << message << "error:" << ret.error().message();
    }
    
    return ret;
}
