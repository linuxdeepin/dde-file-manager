// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESYSTEMHELPER_H
#define FILESYSTEMHELPER_H

#include "diskencrypt_global.h"
FILE_ENCRYPT_BEGIN_NS

namespace filesystem_helper {
bool shrinkFileSystem_ext(const QString &device);
bool expandFileSystem_ext(const QString &device);

bool moveFsForward(const QString &dev);

void remountBoot();
}   // namespace filesystem_helper

FILE_ENCRYPT_END_NS
#endif   // FILESYSTEMHELPER_H
