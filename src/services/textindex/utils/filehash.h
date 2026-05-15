// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEHASH_H
#define FILEHASH_H

#include "service_textindex_global.h"

#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace FileHash {

/**
 * @brief Compute MD5 checksum of a file
 * @param filePath Absolute path to the file
 * @return MD5 hex digest string (32 lowercase chars), or empty string on failure
 */
QString computeMd5(const QString &filePath);

}   // namespace FileHash

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FILEHASH_H
