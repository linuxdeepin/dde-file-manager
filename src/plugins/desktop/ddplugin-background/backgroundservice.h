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
    int currentWorkspaceIndex = 0;
    WMInter *wmInter = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDSERVICE_H
