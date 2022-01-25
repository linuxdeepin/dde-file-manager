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
#ifndef CANVASGRID_P_H
#define CANVASGRID_P_H

#include "canvasgrid.h"
#include "gridcore.h"

#include <QTimer>

DSB_D_BEGIN_NAMESPACE

class CanvasGridPrivate : public QObject, public GridCore
{
    Q_OBJECT
public:
    explicit CanvasGridPrivate(CanvasGrid *qq);
    void sequence(QStringList sortedItems);
    void restore(QStringList currentItems);
    void requestSync();
    void clean();
protected slots:
    void sync();

protected:
    QHash<int, QHash<QString, QPoint>> profiles() const;

public:
    CanvasGrid::Mode mode = CanvasGrid::Mode::Custom;
private:
    CanvasGrid *q;
    QTimer syncTimer;
};

DSB_D_END_NAMESPACE

#endif // CANVASGRID_P_H
