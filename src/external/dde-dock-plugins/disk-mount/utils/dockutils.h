// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKUTILS_H
#define DOCKUTILS_H

#include <QString>

namespace size_format {
QString formatDiskSize(const quint64 num);
QString sizeString(const QString &str);
}

namespace smb_utils {
bool parseSmbInfo(const QString &id, QString &host, QString &share);
}

#endif   // DOCKUTILS_H
