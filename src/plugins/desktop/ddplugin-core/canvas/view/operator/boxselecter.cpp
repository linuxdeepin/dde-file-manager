/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "boxselecter.h"

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QDebug>

DSB_D_USE_NAMESPACE
class BoxSelecterGlobal : public BoxSelecter{};
Q_GLOBAL_STATIC(BoxSelecterGlobal, boxSelecterGlobal)

BoxSelecter *BoxSelecter::instance()
{
    return boxSelecterGlobal;
}

void BoxSelecter::beginSelect(const QPoint &globalPos, bool autoSelect)
{
    begin = globalPos;
    end = globalPos;
    active = true;

    if (automatic = autoSelect)
        qApp->installEventFilter(this);
}

void BoxSelecter::setAcvite(bool ac)
{
    if (ac == active)
        return;

    active = ac;
    emit changed();
}

void BoxSelecter::setBegin(const QPoint &globalPos)
{
    if (globalPos == begin)
        return;

    begin = globalPos;
    emit changed();
}

void BoxSelecter::setEnd(const QPoint &globalPos)
{
    if (globalPos == end)
        return;

    end = globalPos;
    emit changed();
}

QRect BoxSelecter::validRect(QWidget *w) const
{
    QRect selectRect;
    if (!w)
        return selectRect;

    auto rect = globalRect();
    // cover to relative rect to widget w.
    selectRect.setTopLeft(w->mapFromGlobal(rect.topLeft()));
    selectRect.setBottomRight(w->mapFromGlobal(rect.bottomRight()));

    // clip area out of widget.
    return clipRect(selectRect, w->geometry());
}

QRect BoxSelecter::globalRect() const
{
    QRect selectRect;
    selectRect.setLeft(qMin(end.x(), begin.x()));
    selectRect.setTop(qMin(end.y(), begin.y()));
    selectRect.setRight(qMax(end.x(), begin.x()));
    selectRect.setBottom(qMax(end.y(), begin.y()));
    selectRect = selectRect.normalized();
    return selectRect;
}

QRect BoxSelecter::clipRect(QRect rect, const QRect &geometry) const
{
    if (rect.left() < geometry.left())
        rect.setLeft(geometry.left());

    if (rect.right() > geometry.right())
        rect.setRight(geometry.right());

    if (rect.top() < geometry.top())
        rect.setTop(geometry.top());

    if (rect.bottom() > geometry.bottom())
        rect.setBottom(geometry.bottom());

    return rect;
}

bool BoxSelecter::isBeginFrom(QWidget *w)
{
    if (!w)
        return false;

    return w->geometry().contains(w->mapFromGlobal(begin));
}

void BoxSelecter::endSelect()
{
    if (!active)
        return;

    active = false;
    qApp->removeEventFilter(this);
    emit changed();
}

BoxSelecter::BoxSelecter(QObject *parent) : QObject(parent)
{

}

bool BoxSelecter::eventFilter(QObject *watched, QEvent *event)
{
    if (active) {
        switch (event->type()) {
        case QEvent::MouseButtonRelease:
        {
            endSelect();
            // using queue event to prevent disrupting the event cycle
            QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
        }
            break;
        case QEvent::MouseMove:
        {
            QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
            end = e->globalPos();
            QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
        }
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}
