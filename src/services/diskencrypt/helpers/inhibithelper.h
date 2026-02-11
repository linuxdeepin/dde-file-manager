// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INHIBITHELPER_H
#define INHIBITHELPER_H

#include "diskencrypt_global.h"

#include <QDBusUnixFileDescriptor>
#include <QDBusReply>

FILE_ENCRYPT_BEGIN_NS

namespace inhibit_helper {
QDBusReply<QDBusUnixFileDescriptor> inhibit(const QString &message);
}   // namespace inhibit_helper

FILE_ENCRYPT_END_NS

#endif   // INHIBITHELPER_H
