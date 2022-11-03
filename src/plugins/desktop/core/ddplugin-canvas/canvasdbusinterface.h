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
#ifndef CANVASDBUSINTERFACE_H
#define CANVASDBUSINTERFACE_H

#include <QObject>
#include <QDBusContext>

namespace ddplugin_canvas {

class CanvasManager;
class CanvasDBusInterface : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit CanvasDBusInterface(CanvasManager *parent = nullptr);

signals:

public slots:
    void EnableUIDebug(bool enable);
    void Refresh(bool silent = true);
private:
    CanvasManager *manager = nullptr;
};
}
#endif // CANVASDBUSINTERFACE_H
