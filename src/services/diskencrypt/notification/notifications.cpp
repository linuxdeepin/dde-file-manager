// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "notifications.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QString>
#include <QDebug>

FILE_ENCRYPT_USE_NS

SignalEmitter *SignalEmitter::instance()
{
    static SignalEmitter ins;
    return &ins;
}
