#ifndef DBOOKMARKITEM_H
#define DBOOKMARKITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QFont>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#define MARGIN_IMAGE_BOUND 5

class DBookmarkItemGroup;

class DBookmarkItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    DBookmarkItem();

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void boundImageToPress(QString imagePath);
    void boundImageToRelease(QString imagePath);
    void boundImageToHover(QString imagePath);
    void setDraggable(bool b);
    void setBounds(int x, int y, int w, int h);
    void setPressBackgroundColor(const QColor &color);
    void setReleaseBackgroundColor(const QColor &color);
    void setHoverBackgroundColor(const QColor &color);
    void setHoverEnableFlag(bool flag);
    void setAdjust(qreal value);
    bool isPressed();
    bool isHovered();
    void setBackgroundEnable(bool flag);
    int boundX();
    int boundY();
    int boundWidth();
    int boundHeight();
    void setText(const QString & text);
    QString text();
    void setTextColor(const QColor &color);
    QColor getTextColor();
    void setPress(bool b);
    void setPressBackgroundEnable(bool b);
    void setReleaseBackgroundEnable(bool b);
    void setHoverBackgroundEnable(bool b);
    void setUrl(const QString &url);
    QString getUrl();
    void setCheckable(bool b);
    void setChecked(bool b);
    bool isChecked();
    void setItemGroup(DBookmarkItemGroup * group);
    void setWidth(double w);
    void setHeight(double h);
    void setDefaultItem(bool v);
    bool isDefaultItem();
    QPixmap toPixmap() const;
signals:

    void clicked();
    void doubleClicked();
    void url(const QString &url);

    void dragEntered();
    void dragLeft();
    void dropped();
    void dragFinished(const QPointF &point, DBookmarkItem * item);

protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
private:
    bool m_isDefault;
    bool m_checkable;
    bool m_checked;
    bool m_pressed;
    bool m_hovered;
    bool m_backgroundEnabled;

    QColor m_hoverBackgroundColor;
    QColor m_releaseBackgroundColor;
    QColor m_pressBackgroundColor;

    QColor m_textColor;
    QColor m_backGroundColor;

    int m_x_axis;
    int m_y_axis;
    int m_width;
    int m_height;
    int m_text_x;
    int m_text_y;
    qreal m_adjust;
    QPixmap m_pressImage;
    QPixmap m_releaseImage;
    QPixmap m_hoverImage;
    QString m_textContent;

    int m_xOffset, m_yOffset;
    int m_xPos, m_yPos;
    int m_xPress, m_yPress;
    int m_xOffsetImage, m_yOffsetImage;

    bool m_isDraggable;
    bool m_pressBackgroundEnabled;
    bool m_releaseBackgroundEnabled;
    bool m_hoverBackgroundEnabled;
    QFont m_font;
    QString m_url;
    DBookmarkItemGroup * m_group = NULL;
};

#endif // DBOOKMARKITEM_H
