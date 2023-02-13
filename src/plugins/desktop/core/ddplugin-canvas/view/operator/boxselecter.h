// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOXSELECTER_H
#define BOXSELECTER_H

#include "ddplugin_canvas_global.h"

#include <QRect>
#include <QPoint>
#include <QObject>

class QItemSelection;

namespace ddplugin_canvas {
class CanvasView;
class BoxSelecter : public QObject
{
    Q_OBJECT
public:
    static BoxSelecter *instance();
    void beginSelect(const QPoint &globalPos, bool autoSelect);
    void endSelect();
    void setBegin(const QPoint &globalPos);
    void setEnd(const QPoint &globalPos);
    QRect validRect(CanvasView *) const;
    QRect globalRect() const;
    QRect clipRect(QRect rect, const QRect &geometry) const;
    bool isBeginFrom(CanvasView *w);
    void selection(CanvasView *w, const QRect &rect, QItemSelection *newSelection);
public:
    inline bool isAcvite() const {
        return active;
    }
protected:
    void setAcvite(bool ac);
    explicit BoxSelecter(QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
protected:
    virtual void updateSelection();
    virtual void updateCurrentIndex();
private:
    void selection(QItemSelection *newSelection);
    QRect innerGeometry(QWidget *w) const;
signals:
    void changed();
public slots:
private:
    bool automatic = false;
    bool active = false;
    QPoint begin;
    QPoint end;
};

#define BoxSelIns DDP_CANVAS_NAMESPACE::BoxSelecter::instance()

}

#endif // BOXSELECTER_H
