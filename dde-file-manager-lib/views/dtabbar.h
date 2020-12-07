/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DTABBAR_H
#define DTABBAR_H

#include "singleton.h"
#include "durl.h"
#include <QDebug>
#include "singleton.h"
#include <QFont>
#include <QFontMetrics>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QPainter>
#include <QPainterPath>
#include "dfileview.h"
#include "dfmevent.h"

#define TAB_CLOSE_BUTTON_WIDTH 18
#define TAB_CLOSE_BUTTON_HEIGHT 24
#define TAB_MAX_COUNT 8

class Tab: public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Tab(QGraphicsObject *parent = nullptr, DFMBaseView *view = nullptr);
    ~Tab() override;
    void initConnect();
    void setTabText(QString text);
    QString tabText();
    DFMBaseView *fileView();
    void setFileView(DFMBaseView *view);
    DUrl currentUrl() const;
    void setCurrentUrl(const DUrl &url);

    void setFixedSize(QSize size);
    void setGeometry(QRect rect);
    QRect geometry();
    int width();
    int height();
    bool isDragging();
    void setHovered(bool hovered);
    bool isDragOutSide();
    QPixmap toPixmap(bool drawBorder = true);
    bool isChecked() const;
    void setChecked(const bool check);
    bool borderLeft() const;
    void setBorderLeft(const bool flag);
    QString getDisplayNameByUrl(const DUrl &url) const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    void onFileRootUrlChanged(const DUrl &url);

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
signals:
    void clicked();
    void moveNext(Tab *who);
    void movePrevius(Tab *who);
    void requestNewWindow(const DUrl url);
    void aboutToNewWindow(Tab *who);
    void draggingFinished();
    void draggingStarted();
    void requestActiveNextTab();
    void requestActivePreviousTab();

private:
    int m_tabIndex;
    QVariant m_tabData;
    QString m_tabText;

    bool m_hovered{ false };
    bool m_pressed{ false };
    int m_width{ 0 };
    int m_height{ 0 };
    bool m_isDragging{ false };
    QPointF m_originPos;
    bool m_dragOutSide{ false };
    QDrag *m_dragObject{ nullptr };
    bool m_checked{ false };
    bool m_borderLeft{ false };
    DFMBaseView *m_fileView{ nullptr };
    DUrl m_url{};
};

class TabCloseButton: public QGraphicsObject
{
    Q_OBJECT
public:
    explicit TabCloseButton(QGraphicsItem *parent = 0);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    int closingIndex();
    void setClosingIndex(int index);
    void setActiveWidthTab(bool active);
signals:
    void clicked();
    void hovered(int index);
    void unHovered(int index);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool m_mouseHovered{ false };
    bool m_mousePressed{ false };
    int m_closingIndex{ 0 };
    bool m_activeWidthTab{ false };
    QIcon m_active_normalIcon;
    QIcon m_active_hoverIcon;
    QIcon m_active_pressIcon;
    QIcon m_normalIcon;
    QIcon m_hoverIcon;
    QIcon m_pressIcon;
};

class TabBar: public QGraphicsView
{

    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = 0);
    void initConnections();
    int createTab(DFMBaseView *view);
    void removeTab(const int index, const bool &remainState = false);
    void setTabText(const int &index, const QString &text);
    int count() const;
    int currentIndex() const;
    bool tabAddable() const;
    Tab *currentTab();
    Tab *tabAt(const int &index);

signals:
    void currentChanged(const int &index);
    void tabCloseRequested(const int &index, const bool &remainState = false);
    void tabAddableChanged(const bool &tabAddable);
    void tabMoved(const int &from, const int &to);
    void tabBarShown();
    void tabBarHidden();
    void requestNewWindow(const DUrl &url);//###: this signal will connect to Tab::requestNewWindow;

//    void requestCacheRenameBarState();

public slots:
    void setCurrentIndex(const int index);
    void onTabCloseButtonHovered(int closingIndex);
    void onTabCloseButtonUnHovered(int closingIndex);
    void onMoveNext(Tab *who);
    void onMovePrevius(Tab *who);
    void onRequestNewWindow(const DUrl url);
    void onAboutToNewWindow(Tab *who);
    void onTabClicked();
    void onTabDragFinished();
    void onTabDragStarted();
    void activateNextTab();
    void activatePreviousTab();
    void onCurrentUrlChanged(const DFMUrlBaseEvent &event);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    QGraphicsScene *m_scene{ nullptr };
    int m_currentIndex{ -1 };
    QList<Tab *> m_tabs{};
    bool m_barHovered{ false };
    bool m_lastStateOndelete{ false };
    int m_lastDeletedAverageWidth{ 0 };
    QPoint m_tabLastMouseDragPos{};
    bool m_tabDragging{ false };
    TabCloseButton *m_TabCloseButton{ nullptr };
    int m_trackingIndex{ 0 };
    bool m_lastDeleteState{ false };
    bool m_lastAddTabState{ false };
    int m_historyWidth{ 0 };

    QSize tabSizeHint(const int &index);
    void updateScreen();
};




#endif // DTABBAR_H
