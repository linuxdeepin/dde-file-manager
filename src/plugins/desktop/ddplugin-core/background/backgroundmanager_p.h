/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef BACKGROUNDMANAGER_P_H
#define BACKGROUNDMANAGER_P_H

#include "background/backgroundmanager.h"

#include <com_deepin_wm.h>

#include <DWindowManagerHelper>

#include <QObject>

using WMInter = com::deepin::wm;
class QGSettings;
DSB_D_BEGIN_NAMESPACE

class ScreenService;

class BackgroundManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManagerPrivate(BackgroundManager *qq);
    ~BackgroundManagerPrivate();
    QString getBackgroundFormWm(const QString &screen);
    QString getBackgroundFormConfig(const QString &screen);
    QString getDefaultBackground() const;
    bool isEnableBackground();

public:
    BackgroundManager *const q = nullptr;
    WMInter *wmInter = nullptr;
    QGSettings *gsettings = nullptr;
    Dtk::Gui::DWindowManagerHelper* windowManagerHelper = nullptr;
    ScreenService *screenService = nullptr;
    QMap<QString, dfmbase::BackgroundWidgetPointer> backgroundWidgets;
    QMap<QString, QString> backgroundPaths;
    int currentWorkspaceIndex = 1;
};

DSB_D_END_NAMESPACE

#endif // BACKGROUNDMANAGER_P_H
