/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef ABSTRACTSCREENMANAGER_H
#define ABSTRACTSCREENMANAGER_H

#include "abstractscreen.h"
#include <QObject>
#include <QRect>

class ScreenManagerPrivate;
class AbstractScreenManager : public QObject
{
    Q_OBJECT
    friend class ScreenManagerPrivate;
public:
    enum DisplayMode{Custom = 0,Duplicate,Extend,Showonly}; //显示模式
    enum Event{Screen,Mode,Geometry,AvailableGeometry}; //事件类型
public:
    explicit AbstractScreenManager(QObject *parent = nullptr);
    virtual ~AbstractScreenManager();
    virtual ScreenPointer primaryScreen() = 0;
    virtual QVector<ScreenPointer> screens() const = 0; //按接入顺序
    virtual QVector<ScreenPointer> logicScreens() const = 0;    //主屏第一，其他按接入顺序
    virtual ScreenPointer screen(const QString &name) const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;        //从dbus获取后台最新mode
    virtual DisplayMode lastChangedMode() const = 0;    //最后接收到改变后的mode
    virtual void reset() = 0;
protected:
    void appendEvent(Event);    //添加屏幕事件
    virtual void processEvent() = 0;
signals:
    void sigScreenChanged();    //屏幕接入，移除
    void sigDisplayModeChanged();   //显示模式改变
    void sigScreenGeometryChanged(); //屏幕分辨率改变
    void sigScreenAvailableGeometryChanged(); //屏幕可用区改变
protected:
    int m_lastMode = -1;
    ScreenManagerPrivate *d;
};

#endif // ABSTRACTSCREENMANAGER_H
