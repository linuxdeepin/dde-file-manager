// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOXSELECTOR_H
#define BOXSELECTOR_H

#include "ddplugin_canvas_global.h"

#include <QRect>
#include <QPoint>
#include <QObject>

#include <QWidget>
#include <QTimer>

class QItemSelection;

namespace ddplugin_canvas {

class RubberBand : public QWidget
{
    Q_OBJECT
public:
    explicit RubberBand();
    void touch(QWidget *);
protected:
    void paintEvent(QPaintEvent *event) override;
private Q_SLOTS:
    void onParentDestroyed(QObject *);
};

class CanvasView;
class BoxSelector : public QObject
{
    Q_OBJECT
public:
    static BoxSelector *instance();
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
public Q_SLOTS:
    void update();
protected:
    void setAcvite(bool ac);
    explicit BoxSelector(QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
    void delayUpdate();
protected:
    virtual void updateSelection();
    virtual void updateCurrentIndex();
    void updateRubberBand();
private:
    void selection(QItemSelection *newSelection);
    QRect innerGeometry(QWidget *w) const;

private:
    bool automatic = false;
    bool active = false;
    QPoint begin;
    QPoint end;
    RubberBand rubberBand;
    QTimer updateTimer;
};

#define BoxSelIns DDP_CANVAS_NAMESPACE::BoxSelector::instance()
}

#endif // BOXSELECTOR_H
