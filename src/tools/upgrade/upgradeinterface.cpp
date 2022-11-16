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

#include "builtininterface.h"

#include <QDebug>
#include <QCoreApplication>

#include <unistd.h>

using namespace dfm_upgrade;
int dfm_doUpgrade(const QMap<QString, QString> &args)
{
    Q_ASSERT(qApp);

    qInfo() << "upgrade args" << args;

    // is desktop or file manager?
    bool isDesktop = args.contains(kArgDesktop);
    if (!isDesktop && !args.contains(kArgFileManger))
        return -1;

    // check lock
    UpgradeLocker locker;
    if (locker.isLock()) {
        qInfo() << "there are process in upgrading.";
        return 1;
    }

    // check the flag file again.

    // show dialog

    // do upgrade
    UpgradeFactory factor;
    factor.previous(args);
    factor.doUpgrade();
    factor.completed();

    // create flag file

    return 0;
}
