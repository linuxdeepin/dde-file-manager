// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabclosebutton.h"

#include <QIcon>
#include <QGraphicsSceneHoverEvent>

using namespace dfmplugin_workspace;

TabCloseButton::TabCloseButton(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

int TabCloseButton::getClosingIndex()
{
    return closingIndex;
}

void TabCloseButton::setClosingIndex(int index)
{
    closingIndex = index;
}

void TabCloseButton::setActiveWidthTab(bool active)
{
    activeWidthTab = active;
    update();
}

QRectF TabCloseButton::boundingRect() const
{
    return QRectF(0, 0, 24, 24);
}

void TabCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QIcon closeIcon = QIcon::fromTheme("window-close_round");

    QIcon::Mode md = QIcon::Mode::Disabled;
    if (mousePressed) {
        md = QIcon::Mode::Selected;
    }
    if (mouseHovered) {
        md = QIcon::Mode::Active;
    }

    QRect rc = boundingRect().toRect();
    closeIcon.paint(painter, rc, Qt::AlignCenter, md);
}

void TabCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    mousePressed = true;
    if (mouseHovered)
        mouseHovered = false;
    update();
}

void TabCloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    mousePressed = false;
    emit clicked();
    update();
}

void TabCloseButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    mouseHovered = true;
    emit hovered(closingIndex);
    update();
}

void TabCloseButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    if (!mouseHovered)
        mouseHovered = true;
    update();
}

void TabCloseButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unHovered(closingIndex);
    event->ignore();
    mouseHovered = false;
    update();
}
