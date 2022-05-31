/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WINDOWFRAME_P_H
#define WINDOWFRAME_P_H

#include "windowframe.h"
#include "basewindow.h"

#include "interfaces/screen/abstractscreen.h"

#include <QMap>
#include <QReadWriteLock>

DDPCORE_BEGIN_NAMESPACE

class WindowFramePrivate : public QObject
{
    Q_OBJECT
public:
    explicit WindowFramePrivate(WindowFrame *parent);
    void updateProperty(BaseWindowPointer win, DFMBASE_NAMESPACE::ScreenPointer screen, bool primary);
    BaseWindowPointer createWindow(DFMBASE_NAMESPACE::ScreenPointer sp);
public slots:
public:
    QMap<QString, BaseWindowPointer> windows;
    QReadWriteLock locker;
private:
    WindowFrame *q;
};

DDPCORE_END_NAMESPACE

#endif // WINDOWFRAME_P_H
