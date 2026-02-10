// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSINFOUTILS_H
#define SYSINFOUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QMimeData>

namespace dfmbase {
namespace SysInfoUtils {
QString getUser();
QStringList getAllUsersOfHome();
QString getHostName();
QString getOriginalUserHome();
int getUserId();
float getMemoryUsage(int pid);

bool isRootUser();
bool isServerSys();
bool isDesktopSys();
bool isOpenAsAdmin();
bool isDeveloperModeEnabled();
bool isProfessional();
bool isDeepin23();
bool isSameUser(const QMimeData *data);

void setMimeDataUserId(QMimeData *data);
}   // namespace SysInfoUtils
}   // namespace dfmbase

#endif   // SYSINFOUTILS_H
