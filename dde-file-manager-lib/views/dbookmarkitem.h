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
#include "deviceinfo/udiskdeviceinfo.h"
#include "models/bookmark.h"

#define MARGIN_IMAGE_BOUND 5

class DBookmarkItemGroup;
class UDiskDeviceInfo;
class BookMark;
class DDragWidget;
class DFMEvent;
class DBookmarkMountedIndicatorItem;

class DBookmarkItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    DBookmarkItem();

    DBookmarkItem(UDiskDeviceInfoPointer deviceInfo);

    DBookmarkItem(BookMark * bookmark);
    void setDeviceInfo(UDiskDeviceInfoPointer deviceInfo);
    QRectF boundingRect() const;
    void setTightMode(bool v);
    bool isTightModel();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void boundImageToPress(QString imagePath);
    void boundImageToRelease(QString imagePath);
    void boundImageToHover(QString imagePath);
    void boundImageToChecked(QString imagePath);

    void setPressedIcon(const QString& iconPath);
    void setPressedIcon(const QIcon& icon);
    void setHoverIcon(const QString& iconPath);
    void setHoverIcon(const QIcon& icon);
    void setReleaseIcon(const QString& iconPath);
    void setReleaseIcon(const QIcon& icon);
    void setCheckedIcon(const QString& iconPath);
    void setCheckedIcon(const QIcon& icon);


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
    void setHighlightDiskBackgroundColor(const QColor &color);
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
    QString text();
    void setText(const QString & text);

    QColor getTextColor();
    void setTextColor(const QColor &color);

    QColor textHoverColor() const;
    void setTextHoverColor(const QColor &textHoverColor);

    QColor textPressColor() const;
    void setTextPressColor(const QColor &textPressColor);

    QColor textCheckedColor() const;
    void setTextCheckedColor(const QColor &textCheckedColor);

    void setPress(bool b);
    void setPressBackgroundEnable(bool b);
    void setReleaseBackgroundEnable(bool b);
    void setHoverBackgroundEnable(bool b);
    void setHighlightDiskBackgroundEnable(bool b);
    void setHighlightDisk(bool isHighlight);
    bool isHighlightDisk();
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
    bool isDiskItem();
    QPixmap toPixmap();
    bool isMounted();
    void setMounted(bool v);
    void setDeviceLabel(const QString &label);
    QString getDeviceLabel();
    void setDeviceID(const QString &deviceID);
    QString getDeviceID();
    int windowId();
    BookMarkPointer getBookmarkModel();
    void setBookmarkModel(BookMarkPointer bookmark);
    static DBookmarkItem* makeBookmark(const QString &name, const DUrl &url);
    static DBookmarkMountedIndicatorItem* makeMountBookmark(DBookmarkItem* parentItem);
    void editMode();
    void updateMountIndicator();

    bool isMountedIndicator() const;
    void setIsMountedIndicator(bool isMountedIndicator);

    bool getMountBookmark() const;
    void setMountBookmark(bool mountBookmark);

    bool getIsCustomBookmark() const;
    void setIsCustomBookmark(bool isCustomBookmark);

signals:

    void clicked();
    void doubleClicked();

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
    UDiskDeviceInfoPointer m_deviceInfo;
    DBookmarkItemGroup * m_group = NULL;

    DBookmarkMountedIndicatorItem* m_mountBookmarkItem = NULL;

    /* default */
    bool m_isDraggable = false;
    bool m_pressBackgroundEnabled = false;
    bool m_releaseBackgroundEnabled = false;
    bool m_hoverBackgroundEnabled = false;
    bool m_highlightDiskBackgroundEnabled = false;
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
    bool m_isHighlightDisk = false;
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
    QColor m_highlightDiskBackgroundColor;
    QColor m_textColor;
    QColor m_textHoverColor;
    QColor m_textPressColor;
    QColor m_textCheckedColor;
    QColor m_backGroundColor;
    DDragWidget * drag = NULL;
    /* bookmark */
    bool m_isDefault = false;
    bool m_isMountedIndicator = false;

    /* device */
    bool m_isDisk = false;
    bool m_isMounted = false;
    QString m_deviceLabel;
    QString m_deviceID;

    bool m_isCustomBookmark = false;
    bool m_mountBookmark = false;

    /*bookmark model map */
    BookMarkPointer m_bookmarkModel;
public slots:
    void editFinished();
    void checkMountedItem(const DFMEvent& event);
    void playAnimation();
};

#endif // DBOOKMARKITEM_H
