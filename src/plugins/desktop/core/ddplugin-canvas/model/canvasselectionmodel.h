// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASSELECTIONMODEL_H
#define CANVASSELECTIONMODEL_H

#include "ddplugin_canvas_global.h"

#include <QItemSelectionModel>

namespace ddplugin_canvas {
class CanvasProxyModel;
class CanvasSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit CanvasSelectionModel(CanvasProxyModel *model, QObject *parent);
    CanvasProxyModel *model() const;
    QModelIndexList selectedIndexesCache() const;
    QList<QUrl> selectedUrls() const;
public slots:
    void clearSelectedCache();
protected:
    mutable QModelIndexList selectedCache;
};
}
#endif // CANVASSELECTIONMODEL_H
