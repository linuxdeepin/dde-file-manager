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
#ifndef TAB_H
#define TAB_H

#include "dfm-base/dfm_base_global.h"
#include "dfmplugin_workspace_global.h"

#include <QSharedPointer>
#include <QGraphicsObject>

DFMBASE_BEGIN_NAMESPACE
class AbstractBaseView;
DFMBASE_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class TabPrivate;
class Tab : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Tab(QGraphicsObject *parent = nullptr, DFMBASE_NAMESPACE::AbstractBaseView *view = nullptr);

    DFMBASE_NAMESPACE::AbstractBaseView *getCurrentView();
    QUrl getCurrentUrl() const;
    void setCurrentUrl(const QUrl &url);
    void setTabText(const QString &text);
    void setFileView(DFMBASE_NAMESPACE::AbstractBaseView *view);

    bool isChecked() const;
    void setChecked(const bool check);

    int width() const;
    int height() const;
    void setGeometry(const QRect &rect);
    QRect geometry() const;

    bool isDragging() const;
    void setHovered(bool hovered);
    bool isDragOutSide();

    bool borderLeft() const;
    void setBorderLeft(const bool flag);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    void onFileRootUrlChanged(const QUrl &url);

signals:
    void clicked();
    void moveNext(Tab *tab);
    void movePrevius(Tab *tab);
    void requestNewWindow(const QUrl url);
    void aboutToNewWindow(Tab *tab);
    void draggingFinished();
    void draggingStarted();
    void requestActiveNextTab();
    void requestActivePreviousTab();

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QPixmap toPixmap(bool drawBorder) const;

    friend class TabPrivate;
    QSharedPointer<TabPrivate> d;
};

DPWORKSPACE_END_NAMESPACE

#endif   // TAB_H
