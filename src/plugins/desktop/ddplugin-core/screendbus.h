/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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

#include "dfm-base/widgets/abstractscreen.h"
#include "dbus-private/dbusmonitor.h"

#include <QMutex>

class ScreenDBus : public dfmbase::AbstractScreen
{
    Q_OBJECT
    DBusMonitor *dbusMonitor = nullptr;
public:
    explicit ScreenDBus(DBusMonitor *monitor, QObject *parent = nullptr);
    virtual ~ScreenDBus() override;
    bool isValid() const override;
    virtual QString name() const override;
    virtual QRect geometry() const override;
    virtual QRect availableGeometry() const override;
    QRect handleGeometry() const override;
    DBusMonitor *monitor() const;
};

#endif // SCREENDBUS_H
