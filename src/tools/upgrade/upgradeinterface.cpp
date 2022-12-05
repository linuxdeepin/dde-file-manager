/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

