// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundservice.h"

DDP_BACKGROUND_USE_NAMESPACE

BackgroundService::BackgroundService(QObject *parent)
    : QObject(parent)
{
    qInfo() << "create com.deepin.wm";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), this);
    wmInter->setTimeout(200);
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
    qInfo() << "workspace changed " << from << to << "current" << currentWorkspaceIndex;
    currentWorkspaceIndex = to;
    emit backgroundChanged();
}

QString BackgroundService::getDefaultBackground()
{
    return QString("/usr/share/backgrounds/default_background.jpg");
}
