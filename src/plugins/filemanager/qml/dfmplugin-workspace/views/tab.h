// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAB_H
#define TAB_H

#include <dfm-base/dfm_base_global.h>
#include "dfmplugin_workspace_global.h"

#include <QSharedPointer>
#include <QGraphicsObject>

namespace dfmbase {
class AbstractBaseView;
}

namespace dfmplugin_workspace {

class TabPrivate;
class Tab : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Tab(QGraphicsObject *parent = nullptr);

    QUrl getCurrentUrl() const;
    void setCurrentUrl(const QUrl &url);
    void setTabText(const QString &text);
    void setTabAlias(const QString &alias);

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
    QString getDisplayNameByUrl(const QUrl &url) const;

    friend class TabPrivate;
    QSharedPointer<TabPrivate> d;
};

}

#endif   // TAB_H
