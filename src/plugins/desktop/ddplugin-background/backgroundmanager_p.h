/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "backgroundmanager.h"
#include "backgroundservice.h"

#include <com_deepin_wm.h>

#include <QObject>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManagerPrivate(BackgroundManager *qq);
    ~BackgroundManagerPrivate();
    bool isEnableBackground();

    inline QRect relativeGeometry(const QRect &geometry)
    {
        return QRect(QPoint(0, 0), geometry.size());
    }

public:
    BackgroundManager *const q = nullptr;
    BackgroundService *service = nullptr;
    QMap<QString, DFMBASE_NAMESPACE::BackgroundWidgetPointer> backgroundWidgets;
    QMap<QString, QString> backgroundPaths;
    bool enableBackground = true;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDMANAGER_P_H
