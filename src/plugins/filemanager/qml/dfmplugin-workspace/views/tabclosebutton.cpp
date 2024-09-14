// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabclosebutton.h"
#include <dtkgui_global.h>

#include <QIcon>
#include <QGraphicsSceneHoverEvent>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

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
    return QRectF(0, 0, size, size);
}

void TabCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QIcon closeIcon;
    bool isLight = true;

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        isLight = true;
    else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        isLight = false;

    closeIcon = isLight ? QIcon(":/light/icons/tab_close_normal.svg") : QIcon(":/dark/icons/tab_close_normal.svg");

    QIcon::Mode md = QIcon::Mode::Normal;
    if (mousePressed) {
        md = QIcon::Mode::Selected;
        closeIcon = isLight ? QIcon(":/light/icons/tab_close_press.svg") : QIcon(":/dark/icons/tab_close_press.svg");
    }
    if (mouseHovered) {
        md = QIcon::Mode::Active;
        closeIcon = isLight ? QIcon(":/light/icons/tab_close_hover.svg") : QIcon(":/dark/icons/tab_close_hover.svg");
    }

    QRect rc = boundingRect().toRect();
    closeIcon.paint(painter, rc, Qt::AlignTop, md);
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

void TabCloseButton::setSize(int value)
{
    size = value;
}
