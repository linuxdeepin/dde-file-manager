// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INDEXUTILITY_H
#define INDEXUTILITY_H

#include "service_textindex_global.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexUtility {

bool isDefaultIndexedDirectory(const QString &path);
QString statusFilePath();
void removeIndexStatusFile();
void clearIndexDirectory();
void saveIndexStatus(const QDateTime &lastUpdateTime);

}   // namespace IndexUtility

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXUTILITY_H
