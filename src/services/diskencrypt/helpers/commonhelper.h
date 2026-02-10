// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include "diskencrypt_global.h"
FILE_ENCRYPT_BEGIN_NS
namespace common_helper {
QString encryptCipher();
void createDFMDesktopEntry();
void createRebootFlagFile(const QString &dev);
QString genRecoveryKey();
QString genRandomString(int len = 24);
}   // namespace common_helper
FILE_ENCRYPT_END_NS

#endif   // COMMONHELPER_H
