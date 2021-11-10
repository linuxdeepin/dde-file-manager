/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "define.h"

#include <QMutex>

#include "dialogs/dialogmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "deviceinfo/udisklistener.h"
#include "disomaster.h"

Q_GLOBAL_STATIC(DialogManager, gsDialogManager)
Q_GLOBAL_STATIC(GvfsMountManager, gsGvfsMountManager)
Q_GLOBAL_STATIC(UDiskListener, gsUDiskListener)
Q_GLOBAL_STATIC(DISOMasterNS::DISOMaster, gsDISOMaster)

static QMutex odmutex;

bool g_isFileDialogMode = false;

DialogManager *getDialogManager(bool doConstruct)
{
    if (doConstruct) {
        if (!gsDialogManager.exists() && qApp) {
            gsDialogManager->moveToThread(qApp->thread());
        }

        return gsDialogManager();
    }
    return gsDialogManager.exists() ? gsDialogManager() : nullptr;
}

GvfsMountManager *getGvfsMountManager(bool doConstruct)
{
    if (doConstruct) {
        if (!gsGvfsMountManager.exists() && qApp) {
            gsGvfsMountManager->moveToThread(qApp->thread());
        }

        return gsGvfsMountManager();
    }
    return gsGvfsMountManager.exists() ? gsGvfsMountManager() : nullptr;
}

UDiskListener *getUDiskListener(bool doConstruct)
{
    if (doConstruct) {
        if (!gsUDiskListener.exists() && qApp) {
            gsUDiskListener->moveToThread(qApp->thread());
        }

        return gsUDiskListener();
    }
    return gsUDiskListener.exists() ? gsUDiskListener() : nullptr;
}

DISOMasterNS::DISOMaster *getDISOMaster(bool doConstruct)
{
    if (doConstruct) {
        if (!gsDISOMaster.exists() && qApp) {
            gsDISOMaster->moveToThread(qApp->thread());
        }

        return gsDISOMaster();
    }
    return gsDISOMaster.exists() ? gsDISOMaster() : nullptr;
}

QMutex *getOpticalDriveMutex()
{
    return &odmutex;
}
