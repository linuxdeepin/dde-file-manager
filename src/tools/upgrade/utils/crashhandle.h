// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRASHHANDLE_H
#define CRASHHANDLE_H

#include <QObject>

namespace dfm_upgrade {

class CrashHandle
{
public:
    CrashHandle();
    bool isCrashed();
    void clearCrash();
    static void regSignal();
    static void unregSignal();
    static QString upgradeCacheDir();
protected:
    static void handleSignal(int signal);
};

}
#endif // CRASHHANDLE_H
