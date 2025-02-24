// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibithelper.h"

#include <QDBusInterface>
#include <QDebug>

FILE_ENCRYPT_USE_NS

QDBusReply<QDBusUnixFileDescriptor> inhibit_helper::inhibit(const QString &message)
{
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
    qInfo() << message << "inhibited:" << ret.value().fileDescriptor();
    return ret;
}
