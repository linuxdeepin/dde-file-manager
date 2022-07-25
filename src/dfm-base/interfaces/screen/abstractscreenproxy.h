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
#ifndef ABSTRACTSCREENPROXY_H
#define ABSTRACTSCREENPROXY_H

#include "dfm-base/dfm_desktop_defines.h"
#include "interfaces/screen/abstractscreen.h"

#include <QObject>
#include <QMultiMap>

class QTimer;

namespace dfmbase {

class AbstractScreenProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScreenProxy)
public:
    enum Event{Screen,Mode,Geometry,AvailableGeometry};
public:
    explicit AbstractScreenProxy(QObject *parent = nullptr);

    virtual ScreenPointer primaryScreen() = 0;
    virtual QVector<ScreenPointer> screens() const = 0;
    virtual QVector<ScreenPointer> logicScreens() const = 0;
    virtual ScreenPointer screen(const QString &name) const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;
    virtual DisplayMode lastChangedMode() const;
    virtual void reset() = 0;
protected:
    virtual void processEvent() = 0;
protected:
    void appendEvent(Event);
signals:
    void screenChanged();
    void displayModeChanged();
    void screenGeometryChanged();
    void screenAvailableGeometryChanged();
protected:
    DisplayMode lastMode = DisplayMode::kCustom;
    QMultiMap<Event, qint64> events;
private:
    QTimer *eventShot = nullptr;
};

}

#endif // ABSTRACTSCREENPROXY_H
