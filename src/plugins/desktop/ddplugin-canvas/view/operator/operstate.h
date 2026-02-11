// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERSTATE_H
#define OPERSTATE_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QModelIndex>
#include <QItemSelection>

namespace ddplugin_canvas {
class CanvasView;
class OperState : public QObject
{
    Q_OBJECT
public:
    explicit OperState(QObject *parent = nullptr);
    void setView(CanvasView *v);
    QModelIndex current() const;
    void setCurrent(const QModelIndex &value);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    inline QModelIndex getContBegin() const {
        return contBegin;
    }

    inline void setContBegin(const QModelIndex &value) {
        contBegin = value;
    }

protected:
    void updateExpendedItem();
protected:
    CanvasView *view = nullptr;
    QPersistentModelIndex contBegin; //for shift
    QPersistentModelIndex lastExpened;
};
}
#endif // OPERSTATE_H
