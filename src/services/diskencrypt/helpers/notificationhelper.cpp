// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationhelper.h"

FILE_ENCRYPT_USE_NS

NotificationHelper *NotificationHelper::instance()
{
    static NotificationHelper ins;
    return &ins;
}
