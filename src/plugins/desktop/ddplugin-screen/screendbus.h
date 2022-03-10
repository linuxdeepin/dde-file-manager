/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef SCREENDBUS_H
#define SCREENDBUS_H

#include "ddplugin_screen_global.h"

#include <interfaces/screen/abstractscreen.h>

#include <QMutex>

class DBusMonitor;

DDP_SCREEN_BEGIN_NAMESPACE

class ScreenDBus : public DFMBASE_NAMESPACE::AbstractScreen
{
    Q_OBJECT
public:
    explicit ScreenDBus(DBusMonitor *monitor, QObject *parent = nullptr);
    virtual ~ScreenDBus() override;
    virtual QString name() const override;
    virtual QRect geometry() const override;
    virtual QRect availableGeometry() const override;
    QRect handleGeometry() const override;
    QString path() const;
    bool enabled() const;
private:
    DBusMonitor *dbusMonitor = nullptr;
};

DDP_SCREEN_END_NAMESPACE
#endif // SCREENDBUS_H
