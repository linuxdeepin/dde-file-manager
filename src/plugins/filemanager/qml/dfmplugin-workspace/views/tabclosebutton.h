// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABCLOSEBUTTON_H
#define TABCLOSEBUTTON_H

#include "dfmplugin_workspace_global.h"

#include <QGraphicsObject>

namespace dfmplugin_workspace {

class TabCloseButton : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit TabCloseButton(QGraphicsItem *parent = nullptr);
    int getClosingIndex();
    void setClosingIndex(int index);
    void setActiveWidthTab(bool active);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setSize(int value);

signals:
    void clicked();
    void hovered(int index);
    void unHovered(int index);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool mouseHovered = false;
    bool mousePressed = false;
    int closingIndex = 0;
    bool activeWidthTab = false;
    int size = 40;
};

}

#endif   // TABCLOSEBUTTON_H
