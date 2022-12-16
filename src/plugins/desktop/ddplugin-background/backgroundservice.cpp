/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangtingwei<wangtingwei@uniontech.com>
 *
 * Maintainer: wangtingwei<wangtingwei@uniontech.com>
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
#include "backgroundservice.h"

DDP_BACKGROUND_USE_NAMESPACE

BackgroundService::BackgroundService(QObject *parent)
    : QObject(parent)
{
    qInfo() << "create com.deepin.wm";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), this);
    wmInter->setTimeout(1000);
    qInfo() << "create com.deepin.wm end";

    connect(wmInter, &WMInter::WorkspaceSwitched, this, &BackgroundService::onWorkspaceSwitched);
}

BackgroundService::~BackgroundService()
{
    if (wmInter) {
        wmInter->deleteLater();
        wmInter = nullptr;
    }
}

void BackgroundService::onWorkspaceSwitched(int from, int to)
{
    qInfo() << "workspace changed " << from << to;
    currentWorkspaceIndex = to;
    emit backgroundChanged();
}

QString BackgroundService::getDefaultBackground()
{
    return QString("/usr/share/backgrounds/default_background.jpg");
}
