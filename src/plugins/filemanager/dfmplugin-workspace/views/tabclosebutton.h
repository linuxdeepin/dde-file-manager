/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
};

}

#endif   // TABCLOSEBUTTON_H
