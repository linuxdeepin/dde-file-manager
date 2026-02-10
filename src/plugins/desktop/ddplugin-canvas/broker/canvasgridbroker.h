// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRIDBROKER_H
#define CANVASGRIDBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QPoint>

namespace ddplugin_canvas {

class CanvasGrid;
class CanvasGridBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasGridBroker(CanvasGrid *grid, QObject *parent = nullptr);
    ~CanvasGridBroker();
    bool init();

public slots:
    QStringList items(int index);
    QString item(int index, const QPoint &gridPos);
    int point(const QString &item, QPoint *pos);
    void tryAppendAfter(const QStringList &items, int index, const QPoint &begin);
private:
    CanvasGrid *grid = nullptr;
};

}

#endif // CANVASGRIDBROKER_H
