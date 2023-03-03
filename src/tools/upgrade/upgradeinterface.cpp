// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradeinterface.h"
#include "core/upgradelocker.h"
#include "core/upgradefactory.h"
#include "dialog/processdialog.h"
#include "utils/crashhandle.h"

#include "builtininterface.h"

#include <QDebug>
#include <QCoreApplication>

using namespace dfm_upgrade;

int dfm_tools_upgrade_doUpgrade(const QMap<QString, QString> &args)
{
    Q_ASSERT(qApp);

    CrashHandle crash;
    if (crash.isCrashed()) {
        qCritical() << "fail to upgrade, crashed twice.";
        crash.clearCrash();
        QFile::remove(upgradeConfigDir() + "/" + kUpgradeFlag);
        return 0;
    }

    crash.regSignal();

    qInfo() << "upgrade args" << args;

    // is desktop or file manager?
    bool isDesktop = args.contains(kArgDesktop);
    if (!isDesktop && !args.contains(kArgFileManger))
        return -1;

    // check lock
    UpgradeLocker locker;
    if (locker.isLock()) {
        qWarning() << "there is a process in upgrading.";
        return -1;
    }

    // check the flag file again.
    if (!isNeedUpgrade()) {
        qCritical() << "flag file has been removed.";
        return -1;
    }

    // show dialog
    ProcessDialog dlg;
    dlg.initialize(isDesktop);
    if (!dlg.execDialog()) {
        qInfo() << "break by user";
        return -1;
    }

    // do upgrade
    UpgradeFactory factor;
    factor.previous(args);
    factor.doUpgrade();
    factor.completed();

    // remove flag file
    QFile::remove(upgradeConfigDir() + "/" + kUpgradeFlag);
    crash.clearCrash();

    // datas have been upgraded.
    qInfo() << "the upgrader has done.";
    dlg.restart();
    return 0;
}

