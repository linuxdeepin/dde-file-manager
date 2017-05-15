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
#include "ddragwidget.h"
#include "dfileview.h"

#define TAB_CLOSE_BUTTON_WIDTH 18
#define TAB_CLOSE_BUTTON_HEIGHT 24
#define TAB_MAX_COUNT 8

class Tab:public QGraphicsObject{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Tab(QGraphicsObject *parent = 0, DFMBaseView* view = NULL);
    ~Tab();
    void initConnect();
    void setTabText(QString text);
    QString tabText();
    DFMBaseView *fileView();
    void setFileView(DFMBaseView *view);
    DUrl currentUrl() const;
    void setCurrentUrl(const DUrl& url);

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
    QString getDisplayNameByUrl(const DUrl& url) const;

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

public slots:
    void onFileRootUrlChanged(const DUrl& url);

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
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

    bool m_hovered = false;
    bool m_pressed = false;
    int m_width;
    int m_height;
    bool m_isDragging = false;
    QPointF m_originPos;
    bool m_dragOutSide = false;
    QDrag *m_dragObject = NULL;
    bool m_checked = false;
    bool m_borderLeft = false;
    DFMBaseView* m_fileView = NULL;
    DUrl m_url;
};

class TabCloseButton:public QGraphicsObject{
    Q_OBJECT
public:
    explicit TabCloseButton(QGraphicsItem * parent = 0);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) Q_DECL_OVERRIDE;
    int closingIndex();
    void setClosingIndex(int index);
    void setActiveWidthTab(bool active);
signals:
    void clicked();
    void hovered(int index);
    void unHovered(int index);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) Q_DECL_OVERRIDE;

private:
    bool m_mouseHovered = false;
    bool m_mousePressed = false;
    int m_closingIndex;
    bool m_activeWidthTab = false;
};

class TabBar:public QGraphicsView{

    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = 0);
    void initConnections();
    int createTab(DFMBaseView *view);
    void removeTab(const int index, const bool& remainState = false);
    void setTabText(const int& index, const QString&text);
    int count() const;
    int currentIndex() const;
    bool tabAddable() const;
    Tab* currentTab();
    Tab* tabAt(const int& index);

signals:
    void currentChanged(const int &index);
    void tabCloseRequested(const int &index, const bool& remainState = false);
    void tabAddableChanged(const bool &tabAddable);
    void tabMoved(const int &from, const int &to);
    void tabBarShown();
    void tabBarHidden();

public slots:
    void setCurrentIndex(const int index);
    void onTabCloseButtonHovered(int closingIndex);
    void onTabCloseButtonUnHovered(int closingIndex);
    void onMoveNext(Tab* who);
    void onMovePrevius(Tab *who);
    void onRequestNewWindow(const DUrl url);
    void onAboutToNewWindow(Tab *who);
    void onTabClicked();
    void onTabDragFinished();
    void onTabDragStarted();
    void activateNextTab();
    void activatePreviousTab();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    QGraphicsScene *m_scene = NULL;
    int m_currentIndex = -1;
    QList<Tab*> m_tabs;
    bool m_barHovered = false;
    bool m_lastStateOndelete = false;
    int m_lastDeletedAverageWidth = 0;
    QPoint m_tabLastMouseDragPos;
    bool m_tabDragging = false;
    TabCloseButton *m_TabCloseButton;
    int m_trackingIndex = 0;
    bool m_lastDeleteState = false;
    bool m_lastAddTabState = false;
    int m_historyWidth = 0;

    QSize tabSizeHint(const int &index);
    void updateScreen();
};




#endif // DTABBAR_H
