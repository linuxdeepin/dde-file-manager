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
#ifndef CANVASVIEWSHELL_H
#define CANVASVIEWSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

class QMimeData;

namespace ddplugin_organizer {

class CanvasInterface;
class CanvasViewShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewShell(QObject *parent = nullptr);
    ~CanvasViewShell();
    bool initialize();

public:
    QPoint gridPos(const int &viewIndex, const QPoint &viewPoint);

signals: // unqiue and direct signals
    bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPos);

private slots:
    bool eventDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData);
};

}

#endif // CANVASVIEWSHELL_H
