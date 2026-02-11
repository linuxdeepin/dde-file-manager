// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSTABHELPER_H
#define FSTABHELPER_H

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

namespace fstab_helper {
bool setFstabPassno(const QString &devUUID, int passno);
bool setFstabTimeout(const QString &devPath, const QString &devUUID);
}   // namespace fstab_helper

FILE_ENCRYPT_END_NS

#endif   // FSTABHELPER_H
