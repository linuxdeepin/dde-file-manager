#ifndef DTABBAR_H
#define DTABBAR_H

#include <QTabBar>
#include <QWidget>
#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"

#include "widgets/singleton.h"
#include "utils/durl.h"
#include <QDebug>
#include "widgets/singleton.h"
#include <QPushButton>
#include <QFont>
#include <QFontMetrics>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QRect>
#include <QGraphicsProxyWidget>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filemanagerapp.h"
#include "../controllers/appcontroller.h"
#include "QCursor"

#define TAB_CLOSE_BUTTON_WIDTH 18
#define TAB_CLOSE_BUTTON_HEIGHT 24

class DragWidget:public QPushButton{
    Q_OBJECT
public:
    explicit DragWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

class Tab:public QGraphicsObject{
    Q_OBJECT
public:
    explicit Tab(QGraphicsObject *parent = 0,int viewIndex = 0,QString text = "");
    void setTabIndex(int index);
    int tabIndex();
    void setTabData(QVariant data);
    QVariant tabData();
    void setTabText(QString text);
    QString tabText();
    void setFixedSize(QSize size);
    void setGeometry(QRect rect);
    QRect geometry();
    int width();
    int height();
    bool isDragging();
    bool isChecked();
    void setChecked(bool check);
    void setHovered(bool hovered);
    bool isDragOutSide();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
signals:
    void clicked();
    void moveNext(const int fromTabIndex);
    void movePrevius(const int fromTabIndex);
    void requestNewWindow(const int tabIndex);
    void draggingFinished();
    void draggingStarted();

private:
    int m_tabIndex;
    QVariant m_tabData;
    QString m_tabText;

    bool m_hovered = false;
    bool m_pressed = false;
    int m_width;
    int m_height;
    bool m_isDragging = false;
    bool m_checked;
    QPointF m_originPos;
    bool m_dragOutSide = false;
    DragWidget *m_dragWidget = NULL;
    QCursor m_cursor;

};

class TabCloseButton:public QGraphicsObject{
    Q_OBJECT
public:
    explicit TabCloseButton(QGraphicsItem * parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    int closingIndex();
    void setClosingIndex(int index);
signals:
    void clicked();
    void hovered(int index);
    void unHovered(int index);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

private:
    bool m_mouseHovered = false;
    bool m_mousePressed = false;
    int m_closingIndex;
};

class TabBar:public QGraphicsView{

    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = 0);
    int addTabWithData(const int &viewIndex, const QString text, const QString url);
    QVariant tabData(const int index);
    int count() const;
    void removeTab(const int index);
    QRect tabRect(const int index) const;
    int currentIndex() const;
    void setCurrentIndex(const int index);
    bool tabAddable();
    void setTabData(const int &index, const QVariant &tabData);
    void setTabText(const int viewIndex, const QString text, const QString url);
    QSize tabSizeHint(const int &index);
    void updateScreen();
    void initConnections();

signals:
    void currentChanged(const int &index);
    void tabCloseRequested(const int &index);
    void tabAddableChanged(const bool &tabAddable);
    void tabMoved(const int &from, const int &to);

public slots:
    void onTabCloseButtonHovered(int closingIndex);
    void onTabCloseButtonUnHovered(int closingIndex);
    void onMoveNext(const int fromTabIndex);
    void onMovePrevius(const int fromTabIndex);
    void onRequestNewWindow(const int tabIndex);


protected:
    void resizeEvent(QResizeEvent *e);
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    QGraphicsScene *m_scene = NULL;
    int m_currentIndex = -1;
    QList<Tab*>     m_tabs;
    bool m_barHovered = false;
    bool m_lastStateOndelete = false;
    int m_lastDeletedAverageWidth = 0;
    QPoint m_tabLastMouseDragPos;
    bool m_tabDragging = false;
    TabCloseButton *m_TabCloseButton;
    int m_trackingIndex = 0;
    bool m_lastDeleteState = false;
};




#endif // DTABBAR_H
