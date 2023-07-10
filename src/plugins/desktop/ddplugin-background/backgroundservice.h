// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDSERVICE_H
#define BACKGROUNDSERVICE_H

#include "ddplugin_background_global.h"

#include <com_deepin_wm.h>

#include <QGSettings>
#include <QObject>

using WMInter = com::deepin::wm;

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundService : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundService(QObject *parent = nullptr);
    ~BackgroundService();
    virtual QString background(const QString &screen) = 0;
    virtual QString getDefaultBackground() = 0;

signals:
    void backgroundChanged();

protected slots:
    void onWorkspaceSwitched(int from, int to);

protected:
    int getCurrentWorkspaceIndex();
    int currentWorkspaceIndex = 1; // worksapce index is started with 1.
    WMInter *wmInter = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDSERVICE_H
