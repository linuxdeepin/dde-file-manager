// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSINFOUTILS_H
#define SYSINFOUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QMimeData>

namespace dfmbase {

class SysInfoUtils
{

public:
    static QString getUser();
    static QStringList getAllUsersOfHome();
    static QString getHostName();
    static int getUserId();

    static bool isRootUser();
    static bool isServerSys();
    static bool isDesktopSys();
    static bool isOpenAsAdmin();
    static bool isDeveloperModeEnabled();
    static bool isProfessional();
    static bool isSameUser(const QMimeData *data);
    static void setMimeDataUserId(QMimeData *data);
    static float getMemoryUsage(int pid);
};
}

#endif   // SYSINFOUTILS_H
