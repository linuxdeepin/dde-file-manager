// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundservice.h"

#include <QStandardPaths>
#include <QSettings>
#include <QDBusInterface>

DDP_BACKGROUND_USE_NAMESPACE

BackgroundService::BackgroundService(QObject *parent)
    : QObject(parent)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    fmInfo() << "create com.deepin.wm";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), this);
    wmInter->setTimeout(200);
    fmInfo() << "create com.deepin.wm end";
    currentWorkspaceIndex = getCurrentWorkspaceIndex();
    connect(wmInter, &WMInter::WorkspaceSwitched, this, &BackgroundService::onWorkspaceSwitched);
#else
    fmInfo() << "create com.deepin.wm";
    QDBusInterface *interface = new QDBusInterface("com.deepin.wm",
                                                   "/com/deepin/wm",
                                                   "com.deepin.wm",
                                                   QDBusConnection::sessionBus(),
                                                   this);
    fmInfo() << "create com.deepin.wm end";
    interface->setTimeout(200);
    currentWorkspaceIndex = getCurrentWorkspaceIndex();
    connect(interface, SIGNAL(WorkspaceSwitched(int, int)),
            this, SLOT(onWorkspaceSwitched(int, int)));

#endif
}

BackgroundService::~BackgroundService()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (wmInter) {
        wmInter->deleteLater();
        wmInter = nullptr;
    }
#endif
}

void BackgroundService::onWorkspaceSwitched(int from, int to)
{
    fmInfo() << "workspace changed " << from << to << "current" << currentWorkspaceIndex;
    currentWorkspaceIndex = to;
    emit backgroundChanged();
}

QString BackgroundService::getDefaultBackground()
{
    return QString("/usr/share/backgrounds/default_background.jpg");
}

int BackgroundService::getCurrentWorkspaceIndex()
{
    QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/kwinrc";
    QSettings settings(configPath, QSettings::IniFormat);

    bool ok = false;
    int currentIdx = settings.value("Workspace/CurrentDesktop", 1).toInt(&ok);
    fmInfo() << "get currentWorkspaceIndex form config : " << currentIdx;

    if (!ok || currentIdx < 1) {
        currentIdx = 1;
        fmWarning() << "No CurrentWorkspaceIndex obtained, Check if the configuration file has changed";
    }

    return currentIdx;
}
