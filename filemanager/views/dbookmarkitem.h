#ifndef DBOOKMARKITEM_H
#define DBOOKMARKITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QFont>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#define MARGIN_IMAGE_BOUND 5

class DBookmarkItemGroup;
class DeviceInfo;
class BookMark;

class DBookmarkItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    DBookmarkItem();

    DBookmarkItem(DeviceInfo * deviceInfo);

    DBookmarkItem(BookMark * bookmark);

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
    bool isMounted();
    void setMounted(bool v);
    void setDeviceLabel(const QString &label);
    QString getDeviceLabel();
    void setSysPath(const QString &path);
    QString getSysPath();
    int windowId();
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
    void init();

    DBookmarkItemGroup * m_group = NULL;

    /* default */
    bool m_isDraggable = false;
    bool m_pressBackgroundEnabled = false;
    bool m_releaseBackgroundEnabled = false;
    bool m_hoverBackgroundEnabled = false;
    int m_xOffset = 0;
    int m_yOffset = 0;
    int m_xPos = 0;
    int m_yPos = 0;
    int m_xPress = 0;
    int m_yPress = 0;
    int m_xOffsetImage = 0;
    int m_yOffsetImage = 0;
    int m_x_axis = 0;
    int m_y_axis = 0;
    int m_width = 0;
    int m_height = 0;
    int m_text_x = 0;
    int m_text_y = 0;
    bool m_checkable = false;
    bool m_checked = false;
    bool m_pressed = false;
    bool m_hovered = false;
    bool m_backgroundEnabled = false;
    bool m_isMenuOpened = false;
    QFont m_font;
    qreal m_adjust;
    QString m_url;
    QPixmap m_pressImage;
    QPixmap m_releaseImage;
    QPixmap m_hoverImage;
    QString m_textContent;
    QColor m_hoverBackgroundColor;
    QColor m_releaseBackgroundColor;
    QColor m_pressBackgroundColor;
    QColor m_textColor;
    QColor m_backGroundColor;

    /* bookmark */
    bool m_isDefault = false;

    /* device */
    bool m_isDisk = false;
    bool m_isMounted = false;
    QString m_deviceLabel;
    QString m_sysPath;
};

#endif // DBOOKMARKITEM_H
