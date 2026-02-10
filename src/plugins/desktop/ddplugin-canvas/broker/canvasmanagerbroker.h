// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGERBROKER_H
#define CANVASMANAGERBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>

class QAbstractItemModel;
class QAbstractItemView;
class QItemSelectionModel;

namespace ddplugin_canvas {
class CanvasManager;
class CanvasManagerBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasManagerBroker(CanvasManager *canvas, QObject *parent = nullptr);
    ~CanvasManagerBroker();
    bool init();
public slots:
    void update();
    void edit(const QUrl &url);
    int iconLevel();
    void setIconLevel(int lv);
    bool autoArrange();
    void setAutoArrange(bool on);
    QAbstractItemModel *fileInfoModel();
    QAbstractItemView *view(int viewIdx);
    QItemSelectionModel *selectionModel();
private:
    CanvasManager *canvas = nullptr;
};

}

#endif // CANVASMANAGERBROKER_H
