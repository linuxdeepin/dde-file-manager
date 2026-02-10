// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PROTOCOLUTILS_H
#define PROTOCOLUTILS_H

#include <dfm-base/dfm_base_global.h>

DFMBASE_BEGIN_NAMESPACE

namespace ProtocolUtils {
bool isLocalFile(const QUrl &url);
bool isRemoteFile(const QUrl &url);
bool isMTPFile(const QUrl &url);
bool isGphotoFile(const QUrl &url);
bool isFTPFile(const QUrl &url);
bool isSFTPFile(const QUrl &url);
bool isSMBFile(const QUrl &url);
bool isNFSFile(const QUrl &url);
bool isDavFile(const QUrl &url);
bool isDavsFile(const QUrl &url);
}   // namespace ProtocolUtils

DFMBASE_END_NAMESPACE

#endif   // PROTOCOLUTILS_H
