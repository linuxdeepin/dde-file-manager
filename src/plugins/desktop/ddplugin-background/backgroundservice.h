// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDSERVICE_H
#define BACKGROUNDSERVICE_H

#include "ddplugin_background_global.h"

#include <QObject>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <com_deepin_wm.h>
using WMInter = com::deepin::wm;
#endif

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
    int currentWorkspaceIndex = 1;   // workspace index is started with 1.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    WMInter *wmInter = nullptr;
#endif
};

DDP_BACKGROUND_END_NAMESPACE

#endif   // BACKGROUNDSERVICE_H
