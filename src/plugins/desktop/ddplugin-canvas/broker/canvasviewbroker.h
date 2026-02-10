// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEWBROKER_H
#define CANVASVIEWBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QRect>

namespace ddplugin_canvas {
class CanvasManager;
class CanvasView;
class CanvasViewBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewBroker(CanvasManager *, QObject *parent = nullptr);
    ~CanvasViewBroker();
    bool init();
    QSharedPointer<CanvasView> getView(int idx);
public slots:
    QRect visualRect(int idx, const QUrl &url);
    QRect gridVisualRect(int idx, const QPoint &gridPos);
    QPoint gridPos(int idx, const QPoint &viewPoint);
    QSize gridSize(int idx);
    void refresh(int idx);
    void update(int idx);
    void select(const QList<QUrl> &urls);
    QList<QUrl> selectedUrls(int idx);
    QObject *fileOperator();
public:
    QRect iconRect(int idx, QRect visualRect);
private:
    CanvasManager *manager = nullptr;
};

}

#endif // CANVASVIEWBROKER_H
