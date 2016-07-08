#ifndef DBOOKMARKITEM_H
#define DBOOKMARKITEM_H

#include <QObject>
#include <QGraphicsWidget>
#include <QGraphicsTextItem>
#include <QFont>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>

#include "durl.h"

#define MARGIN_IMAGE_BOUND 5

class DBookmarkItemGroup;
class UDiskDeviceInfo;
class BookMark;
class DDragWidget;
class FMEvent;
class DBookmarkMountedIndicatorItem;

class DBookmarkItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    DBookmarkItem();

    DBookmarkItem(UDiskDeviceInfo * deviceInfo);

    DBookmarkItem(BookMark * bookmark);
    void setDeviceInfo(UDiskDeviceInfo * deviceInfo);
    QRectF boundingRect() const;
    void setTightMode(bool v);
    bool isTightModel();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void boundImageToPress(QString imagePath);
    void boundImageToRelease(QString imagePath);
    void boundImageToHover(QString imagePath);
    void boundImageToChecked(QString imagePath);

    QPixmap getCheckedPixmap();

    void boundBigImageToPress(QString imagePath);
    void boundBigImageToRelease(QString imagePath);
    void boundBigImageToHover(QString imagePath);
    void boundBigImageToChecked(QString imagePath);
    void setDraggable(bool b);
    void setBounds(int x, int y, int w, int h);
    void setPressBackgroundColor(const QColor &color);
    void setReleaseBackgroundColor(const QColor &color);
    void setHoverBackgroundColor(const QColor &color);
    void setHoverEnableFlag(bool flag);
    void setAdjust(qreal value);
    bool isPressed();
    bool isHovered();
    void setHovered(bool flag);
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
    void setUrl(const DUrl &url);
    DUrl getUrl();
    void setCheckable(bool b);
    void setChecked(bool b);
    bool isChecked();
    void setItemGroup(DBookmarkItemGroup * group);
    void setWidth(double w);
    void setHeight(double h);
    void setDefaultItem(bool v);
    bool isDefaultItem();
    QPixmap toPixmap();
    bool isMounted();
    void setMounted(bool v);
    void setDeviceLabel(const QString &label);
    QString getDeviceLabel();
    void setSysPath(const QString &path);
    QString getSysPath();
    int windowId();
    static DBookmarkItem* makeBookmark(const QString &name, const DUrl &url);
    static DBookmarkMountedIndicatorItem* makeMountBookmark(DBookmarkItem* parentItem);
    void editMode();
    void updateMountIndicator();
signals:

    void clicked();
    void doubleClicked();
    void url(const QString &url);

    void dragEntered();
    void dragLeft();
    void dropped();
    void dragFinished(const QPointF &point, const QPointF &scenePoint, DBookmarkItem * item);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
private:
    void init();
    QLineEdit * m_lineEdit;
    QGraphicsProxyWidget * m_widget = NULL;
    UDiskDeviceInfo * m_deviceInfo = NULL;
    DBookmarkItemGroup * m_group = NULL;

    DBookmarkMountedIndicatorItem* m_mountBookmarkItem = NULL;

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
    bool m_isTightMode = false;
    QFont m_font;
    qreal m_adjust = 0;
    DUrl m_url;
    QPixmap m_pressImage;
    QPixmap m_releaseImage;
    QPixmap m_hoverImage;
    QPixmap m_checkedImage;
    QPixmap m_pressImageBig;
    QPixmap m_releaseImageBig;
    QPixmap m_hoverImageBig;
    QPixmap m_checkedImageBig;
    QString m_textContent;
    QColor m_hoverBackgroundColor;
    QColor m_releaseBackgroundColor;
    QColor m_pressBackgroundColor;
    QColor m_textColor;
    QColor m_backGroundColor;
    DDragWidget * drag = NULL;
    /* bookmark */
    bool m_isDefault = false;

    /* device */
    bool m_isDisk = false;
    bool m_isMounted = false;
    QString m_deviceLabel;
    QString m_sysPath;
public slots:
    void editFinished();
    void checkMountedItem(const FMEvent& event);
};

#endif // DBOOKMARKITEM_H
