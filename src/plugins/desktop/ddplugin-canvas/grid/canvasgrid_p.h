// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRID_P_H
#define CANVASGRID_P_H

#include "canvasgrid.h"
#include "gridcore.h"

#include <QTimer>

namespace ddplugin_canvas {

class CanvasGridPrivate : public QObject, public GridCore
{
    Q_OBJECT
public:
    explicit CanvasGridPrivate(CanvasGrid *qq);
    void sequence(QStringList sortedItems);
    void restore(QStringList currentItems);
    void clean();
protected slots:
    void sync();

protected:
    QHash<int, QHash<QString, QPoint>> profiles() const;

public:
    CanvasGrid::Mode mode = CanvasGrid::Mode::Custom;
    QTimer syncTimer;
private:
    CanvasGrid *q;
};

}

#endif // CANVASGRID_P_H
