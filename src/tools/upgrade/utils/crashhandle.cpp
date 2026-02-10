// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crashhandle.h"

#include <QStandardPaths>
#include <QFile>
#include <QLoggingCategory>

#include <signal.h>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

// crashed twice
inline constexpr char kCrashFlag0[] = "dfm-upgraded.crash.0";
inline constexpr char kCrashFlag1[] = "dfm-upgraded.crash.1";

using namespace dfm_upgrade;

CrashHandle::CrashHandle()
{
}

void CrashHandle::regSignal()
{
    signal(SIGTERM, CrashHandle::handleSignal);
    signal(SIGPIPE, CrashHandle::handleSignal);
    signal(SIGILL, CrashHandle::handleSignal);
    signal(SIGABRT, CrashHandle::handleSignal);
    signal(SIGSEGV, CrashHandle::handleSignal);
    signal(SIGTRAP, CrashHandle::handleSignal);
    signal(SIGBUS, CrashHandle::handleSignal);
    signal(SIGSYS, CrashHandle::handleSignal);
}

void CrashHandle::unregSignal()
{
    signal(SIGTERM, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGSYS, SIG_DFL);
}

bool CrashHandle::isCrashed()
{
    return QFile::exists(upgradeCacheDir() + "/" + kCrashFlag0)
            && QFile::exists(upgradeCacheDir() + "/" + kCrashFlag1);
}

void CrashHandle::clearCrash()
{
    QFile::remove(upgradeCacheDir() + "/" + kCrashFlag0);
    QFile::remove(upgradeCacheDir() + "/" + kCrashFlag1);
}

QString CrashHandle::upgradeCacheDir()
{
    return QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first() + "/deepin/dde-file-manager";
}

void CrashHandle::handleSignal(int signal)
{
    unregSignal();

    QString needCreate = upgradeCacheDir() + "/" + kCrashFlag0;
    if (QFile::exists(needCreate))
        needCreate = upgradeCacheDir() + "/" + kCrashFlag1;

    QFile file(needCreate);
    file.open(QFile::NewOnly);
    file.close();

    qCCritical(logToolUpgrade) << "upragde crash" << signal;
    raise(signal);
}
