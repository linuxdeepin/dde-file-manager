// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FSRESIZE_H
#define FSRESIZE_H

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

namespace fs_resize {
bool shrinkFileSystem(const QString &device);
bool expandFileSystem(const QString &device);

bool shrinkFileSystem_ext(const QString &device);
bool expandFileSystem_ext(const QString &device);
bool recoverySuperblock_ext(const QString &device, const QString &cryptHeaderPath);
}   // namespace fs_resize

FILE_ENCRYPT_END_NS

#endif   // FSRESIZE_H
