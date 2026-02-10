// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASSELECTIONMODEL_H
#define CANVASSELECTIONMODEL_H

#include "ddplugin_canvas_global.h"

#include <QItemSelectionModel>

namespace ddplugin_canvas {
class CanvasProxyModel;
class SelectionHookInterface;
class CanvasSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit CanvasSelectionModel(CanvasProxyModel *model, QObject *parent);
    inline void setHook(SelectionHookInterface *ifs) {
        hook = ifs;
    }
    CanvasProxyModel *model() const;
    QModelIndexList selectedIndexesCache() const;
    QList<QUrl> selectedUrls() const;
    void selectAll();
    void hookClear();
public slots:
    void clearSelectedCache();
    void clear() override;
protected:
    mutable QModelIndexList selectedCache;
private:
    SelectionHookInterface *hook = nullptr;
};
}
#endif // CANVASSELECTIONMODEL_H
