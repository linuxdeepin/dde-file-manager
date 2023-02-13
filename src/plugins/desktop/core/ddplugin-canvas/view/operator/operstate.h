// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERSTATE_H
#define OPERSTATE_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QModelIndex>

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

    inline QModelIndex getContBegin() const {
        return contBegin;
    }

    inline void setContBegin(const QModelIndex &value) {
        contBegin = value;
    }
signals:

public slots:
protected slots:
    void selectionChanged();
protected:
    CanvasView *view = nullptr;
    QPersistentModelIndex contBegin; //for shift
};
}
#endif // OPERSTATE_H
